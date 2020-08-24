/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "attribute_edit.h"
#include "attribute_display_strings.h"
#include "utils.h"
#include "details_widget.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QDateTimeEdit>

void layout_attribute_edits(QList<AttributeEdit *> edits, QGridLayout *layout, QWidget *parent) {
    for (auto edit : edits) {
        edit->add_to_layout(layout);
    }
}

bool verify_attribute_edits(QList<AttributeEdit *> edits, QWidget *parent) {
    bool success = true;

    for (auto edit : edits) {
        const bool verify_success = edit->verify_input(parent);

        if (!verify_success) {
            success = false;
        }
    }

    return success;
}

// NOTE: apply and collect results, THEN show error popups, so that all AD requests are done first and there are no stutters between popups
bool apply_attribute_edits(QList<AttributeEdit *> edits, const QString &dn, ApplyAttributeEditBatch batch, QWidget *parent) {
    if (batch == ApplyAttributeEditBatch_Yes) {
        AdInterface::instance()->start_batch();
    }

    bool success = true;

    QList<AdResult> results;
    for (auto edit : edits) {
        const AdResult result = edit->apply(dn);
        results.append(result);
    }

    for (auto result : results) {
        if (!result.success) {
            success = false;

            QMessageBox::critical(parent, QObject::tr("Error"), result.error_with_context);
        }
    }

    if (batch == ApplyAttributeEditBatch_Yes) {
        AdInterface::instance()->end_batch();
    }

    return success;
}

bool apply_attribute_edit(AttributeEdit *edit, const QString &dn, QWidget *parent) {
    QList<AttributeEdit* > edits;
    edits.append(edit);

    const bool apply_success = apply_attribute_edits(edits, dn, ApplyAttributeEditBatch_No, parent);

    return apply_success;
}

void make_string_edits(const QList<QString> attributes, QMap<QString, StringEdit *> *edits_out) {
    for (auto attribute : attributes) {
        auto edit = new StringEdit(attribute);
        edits_out->insert(attribute, edit);
    }
}

QMap<AccountOption, AccountOptionEdit *> make_account_option_edits(const QList<AccountOption> options, QWidget *parent) {
    QMap<AccountOption, AccountOptionEdit *> edits;

    for (auto option : options) {
        auto edit = new AccountOptionEdit(option);
        edits.insert(option, edit);
    }

    // PasswordExpired conflicts with (DontExpirePassword and CantChangePassword)
    // When PasswordExpired is set, the other two can't be set
    // When any of the other two are set, PasswordExpired can't be set
    // Implement this by connecting to state changes of all options and
    // resetting to previous state if state transition is invalid
    auto setup_conflict =
    [parent, edits](const AccountOption subject, const AccountOption blocker) {
        QCheckBox *subject_check = edits[subject]->check;
        QCheckBox *blocker_check = edits[blocker]->check;

        QObject::connect(subject_check, &QCheckBox::stateChanged,
            [subject, blocker, subject_check, blocker_check, parent]() {
                if (checkbox_is_checked(subject_check) && checkbox_is_checked(blocker_check)) {
                    subject_check->setCheckState(Qt::Unchecked);

                    const QString subject_name = get_account_option_description(subject);
                    const QString blocker_name = get_account_option_description(blocker);
                    const QString error = QString(QObject::tr("Can't set \"%1\" when \"%2\" is set.")).arg(blocker_name, subject_name);
                    QMessageBox::warning(parent, QObject::tr("Error"), error);
                }
            }
            );
    };

    // NOTE: only setup conflicts for options that exist
    if (options.contains(AccountOption_PasswordExpired)) {
        const QList<AccountOption> other_two_options = {
            AccountOption_DontExpirePassword,
            // TODO: AccountOption_CantChangePassword
        };

        for (auto other_option : other_two_options) {
            if (options.contains(other_option)) {
                setup_conflict(AccountOption_PasswordExpired, other_option);
                setup_conflict(other_option, AccountOption_PasswordExpired);
            }
        }
    }

    return edits;
}

void connect_edits_to_details(QList<AttributeEdit *> edits, DetailsWidget *details) {
    for (auto edit : edits) {
        edit->connect_to_details(details);
    }
}

StringEdit::StringEdit(const QString &attribute_arg, const EditReadOnly read_only) {
    edit = new QLineEdit();
    attribute = attribute_arg;

    if (read_only == EditReadOnly_Yes) {
        edit->setReadOnly(true);
    }
}

void StringEdit::load(const QString &dn) {
    QString value;
    if (attribute == ATTRIBUTE_OBJECT_CLASS) {
        // NOTE: object class is multi-valued so need to get the "primary" class
        // TODO: not sure how to get the "primary" attribute, for now just getting the last one. I think what I need to do is get the most "derived" class? and that info should be in the scheme.
        const QList<QString> classes = AdInterface::instance()->attribute_get_multi(dn, attribute);
        value = classes.last();
    } else {
        value = AdInterface::instance()->attribute_get(dn, attribute);
    }

    edit->setText(value);
}

void StringEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_attribute_display_string(attribute);
    QWidget *widget = edit;
    append_to_grid_layout_with_label(layout, label_text , widget);
}

void StringEdit::connect_to_details(DetailsWidget *details) const {
    QObject::connect(
        edit, &QLineEdit::textChanged,
        details, &DetailsWidget::on_edit_changed);
}

bool StringEdit::verify_input(QWidget *parent) {
    if (attribute == ATTRIBUTE_SAMACCOUNT_NAME) {
        const QString new_value = edit->text();

        if (new_value.isEmpty()) {
            const QString attribute_string = get_attribute_display_string(attribute);
            const QString error_text = QString(QObject::tr("Attribute \"%1\" cannot be empty!").arg(attribute_string));
            QMessageBox::warning(parent, QObject::tr("Error"), error_text);

            return false;
        }
    }

    return true;
}

AdResult StringEdit::apply(const QString &dn) {
    const QString new_value = edit->text();
    const AdResult result = AdInterface::instance()->attribute_replace(dn, attribute, new_value);

    return result;
}

GroupScopeEdit::GroupScopeEdit() {
    combo = new QComboBox();

    for (int i = 0; i < GroupScope_COUNT; i++) {
        const GroupScope type = (GroupScope) i;
        const QString type_string = group_scope_to_string(type);

        combo->addItem(type_string, (int)type);
    }
}

void GroupScopeEdit::load(const QString &dn) {
    const GroupScope scope = AdInterface::instance()->group_get_scope(dn);
    const int scope_int = (int)scope;
    combo->setCurrentIndex(scope_int);
}

void GroupScopeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = QObject::tr("Group scope");
    QWidget *widget = combo;
    append_to_grid_layout_with_label(layout, label_text , widget);
}

void GroupScopeEdit::connect_to_details(DetailsWidget *details) const {
    QObject::connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        details, &DetailsWidget::on_edit_changed);
}

bool GroupScopeEdit::verify_input(QWidget *parent) {
    return true;
}

AdResult GroupScopeEdit::apply(const QString &dn) {
    const GroupScope new_value = (GroupScope)combo->currentData().toInt();
    const AdResult result = AdInterface::instance()->group_set_scope(dn, new_value);

    return result;
}

GroupTypeEdit::GroupTypeEdit() {
    combo = new QComboBox();

    for (int i = 0; i < GroupType_COUNT; i++) {
        const GroupType type = (GroupType) i;
        const QString type_string = group_type_to_string(type);

        combo->addItem(type_string, (int)type);
    }
}

void GroupTypeEdit::load(const QString &dn) {
    const GroupType type = AdInterface::instance()->group_get_type(dn);
    const int type_int = (int)type;
    combo->setCurrentIndex(type_int);
}

void GroupTypeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = QObject::tr("Group type");
    QWidget *widget = combo;
    append_to_grid_layout_with_label(layout, label_text , widget);
}

void GroupTypeEdit::connect_to_details(DetailsWidget *details) const {
    QObject::connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        details, &DetailsWidget::on_edit_changed);
}

bool GroupTypeEdit::verify_input(QWidget *parent) {
    return true;
}

AdResult GroupTypeEdit::apply(const QString &dn) {
    const GroupType new_value = (GroupType)combo->currentData().toInt();
    const AdResult result = AdInterface::instance()->group_set_type(dn, new_value);

    return result;
}

AccountOptionEdit::AccountOptionEdit(const AccountOption option_arg) {
    option = option_arg;
    check = new QCheckBox();
}

void AccountOptionEdit::load(const QString &dn) {
    const bool option_is_set = AdInterface::instance()->user_get_account_option(dn, option);

    // TODO: block for all? logical i think
    check->blockSignals(true);
    {
        Qt::CheckState check_state;
        if (option_is_set) {
            check_state = Qt::Checked;
        } else {
            check_state = Qt::Unchecked;
        }

        check->setCheckState(check_state);
    }
    check->blockSignals(false);
}

void AccountOptionEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_account_option_description(option);
    QWidget *widget = check;
    append_to_grid_layout_with_label(layout, label_text , widget);
}

void AccountOptionEdit::connect_to_details(DetailsWidget *details) const {
    QObject::connect(
        check, &QCheckBox::stateChanged,
        details, &DetailsWidget::on_edit_changed);
}

bool AccountOptionEdit::verify_input(QWidget *parent) {
    return true;
}

AdResult AccountOptionEdit::apply(const QString &dn) {
    const bool new_value = checkbox_is_checked(check);
    const AdResult result = AdInterface::instance()->user_set_account_option(dn, option, new_value);

    return result;
}

PasswordEdit::PasswordEdit() {
    edit = new QLineEdit();
    confirm_edit = new QLineEdit();

    edit->setEchoMode(QLineEdit::Password);
    confirm_edit->setEchoMode(QLineEdit::Password);
}

void PasswordEdit::load(const QString &dn) {
    // NOTE: PasswordEdit does not load current value, it starts out blank
}

void PasswordEdit::add_to_layout(QGridLayout *layout) {
    append_to_grid_layout_with_label(layout, QObject::tr("Password") , edit);
    append_to_grid_layout_with_label(layout, QObject::tr("Confirm password") , confirm_edit);
}

void PasswordEdit::connect_to_details(DetailsWidget *details) const {

}

bool PasswordEdit::verify_input(QWidget *parent) {
    const QString pass = edit->text();
    const QString confirm_pass = confirm_edit->text();

    if (pass != confirm_pass) {
        const QString error_text = QString(QObject::tr("Passwords don't match!"));
        QMessageBox::warning(parent, QObject::tr("Error"), error_text);

        return false;
    }

    return true;
}

AdResult PasswordEdit::apply(const QString &dn) {
    const QString new_value = edit->text();

    const AdResult result = AdInterface::instance()->set_pass(dn, new_value);

    return result;
}

DateTimeEdit::DateTimeEdit(const QString &attribute_arg, EditReadOnly read_only) {
    edit = new QDateTimeEdit();
    attribute = attribute_arg;

    if (read_only == EditReadOnly_Yes) {
        edit->setReadOnly(true);
    }
}

void DateTimeEdit::load(const QString &dn) {
    const QDateTime value = AdInterface::instance()->attribute_datetime_get(dn, attribute);

    edit->setDateTime(value);
}

void DateTimeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_attribute_display_string(attribute);
    append_to_grid_layout_with_label(layout, label_text, edit);
}

void DateTimeEdit::connect_to_details(DetailsWidget *details) const {
    QObject::connect(
        edit, &QDateTimeEdit::dateTimeChanged,
        details, &DetailsWidget::on_edit_changed);
}

bool DateTimeEdit::verify_input(QWidget *parent) {
    // TODO: datetime should fit within bounds of it's format, so greater than start of epoch for NTFS format?

    return true;
}

AdResult DateTimeEdit::apply(const QString &dn) {
    const QDateTime new_value = edit->dateTime();

    const AdResult result = AdInterface::instance()->attribute_datetime_replace(dn, attribute, new_value);

    return result;
}
