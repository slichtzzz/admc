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

#include "edits/expiry_edit.h"
#include "ad_interface.h"
#include "server_configuration.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDateTime>
#include <QButtonGroup>
#include <QCalendarWidget>
#include <QDialog>
#include <QDialogButtonBox>

#define DATE_FORMAT "dd.MM.yyyy"

const QTime END_OF_DAY(23, 59);

ExpiryEdit::ExpiryEdit(QObject *parent)
: AttributeEdit(parent)
{
    never_check = new QCheckBox(tr("Never"));
    end_of_check = new QCheckBox(tr("End of:"));

    auto button_group = new QButtonGroup();
    button_group->setExclusive(true);
    button_group->addButton(never_check);
    button_group->addButton(end_of_check);
    edit_button = new QPushButton(tr("Edit"));
    display_label = new QLabel();

    connect(
        never_check, &QCheckBox::stateChanged,
        this, &ExpiryEdit::on_never_check);
    connect(
        end_of_check, &QCheckBox::stateChanged,
        this, &ExpiryEdit::on_end_of_check);
    connect(
        edit_button, &QAbstractButton::clicked,
        this, &ExpiryEdit::on_edit_button);
}

void ExpiryEdit::load(const Attributes &attributes) {
    const QString expiry_raw(attributes[ATTRIBUTE_ACCOUNT_EXPIRES][0]);

    original_value = expiry_raw;

    const bool never = datetime_is_never(ATTRIBUTE_ACCOUNT_EXPIRES, expiry_raw);

    // NOTE: prevent setChecked() from emitting signals
    QCheckBox *checks[] = {
        never_check,
        end_of_check
    };
    for (auto check : checks) {
        check->blockSignals(true);
    }
    {
        never_check->setChecked(never);
        end_of_check->setChecked(!never);
    }
    for (auto check : checks) {
        check->blockSignals(false);
    }

    display_label->setEnabled(!never);
    edit_button->setEnabled(!never);

    QString display_label_text;
    if (never) {
        // Load current date as default value when expiry is never
        const QDate default_expiry = QDate::currentDate();
        display_label_text = default_expiry.toString(DATE_FORMAT);
    } else {
        const QDateTime current_expiry = datetime_raw_to_datetime(ATTRIBUTE_ACCOUNT_EXPIRES, expiry_raw);
        display_label_text = current_expiry.toString(DATE_FORMAT);
    }
    display_label->setText(display_label_text);

    emit edited();
}

void ExpiryEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_attribute_display_name(ATTRIBUTE_ACCOUNT_EXPIRES, "") + ":";
    const auto label = new QLabel(label_text);

    layout->addWidget(label, layout->rowCount(), 0);
    layout->addWidget(never_check, layout->rowCount(), 0);
    layout->addWidget(end_of_check, layout->rowCount(), 0);
    layout->addWidget(display_label, layout->rowCount(), 0);
    layout->addWidget(edit_button, layout->rowCount(), 0);

    connect_changed_marker(this, label);
}

// TODO: limit date to the format's date range, make a f-n in adinterface that checks if format in limit
bool ExpiryEdit::verify_input(QWidget *parent) {
    return true;
}

bool ExpiryEdit::changed() const {
    const QString new_value = get_new_value();
    return (new_value != original_value);
}

bool ExpiryEdit::apply(const QString &dn) {
    const QString new_value = get_new_value();
    const bool result = AdInterface::instance()->attribute_replace(dn, ATTRIBUTE_ACCOUNT_EXPIRES, new_value);

    return result;
}

void ExpiryEdit::on_never_check() {
    if (checkbox_is_checked(never_check)) {
        display_label->setEnabled(false);
        edit_button->setEnabled(false);

        emit edited();
    }
}

void ExpiryEdit::on_end_of_check() {
    if (checkbox_is_checked(end_of_check)) {
        display_label->setEnabled(true);
        edit_button->setEnabled(true);

        emit edited();
    }
}

void ExpiryEdit::on_edit_button() {
    auto dialog = new QDialog();

    auto label = new QLabel(tr("Edit expiry time"), dialog);
    auto calendar = new QCalendarWidget(dialog);
    auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok |  QDialogButtonBox::Cancel, dialog);

    const auto layout = new QVBoxLayout(dialog);
    layout->addWidget(label);
    layout->addWidget(calendar);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        [this, dialog, calendar]() {
            const QDate new_date = calendar->selectedDate();
            const QString new_date_string = new_date.toString(DATE_FORMAT);

            display_label->setText(new_date_string);

            dialog->accept();

            emit edited();
        });

    connect(
        button_box, &QDialogButtonBox::rejected,
        [dialog]() {
            dialog->reject();
        });

    dialog->open();
}

// NOTE: have to operate on raw string datetime values here because (never) value can't be expressed as QDateTime
QString ExpiryEdit::get_new_value() const {
    const bool never = checkbox_is_checked(never_check);

    if (never) {
        const bool original_value_is_never = datetime_is_never(ATTRIBUTE_ACCOUNT_EXPIRES, original_value);

        // NOTE: there are two valid NEVER values so need to match original if it was "never" to avoid changing to different NEVER value and entering into "changed" state when nothing was actually changed
        if (original_value_is_never) {
            return original_value;
        } else {
            return AD_LARGEINTEGERTIME_NEVER_1;
        }
    } else {
        const QString new_date_string = display_label->text();
        const QDate new_date = QDate::fromString(new_date_string, DATE_FORMAT);
        const QDateTime new_datetime(new_date, END_OF_DAY);
        const QString value = datetime_to_string(ATTRIBUTE_ACCOUNT_EXPIRES, new_datetime);

        return value;
    }
}
