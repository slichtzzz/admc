/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "edits/logon_hours_edit.h"
#include "edits/logon_hours_edit_p.h"
#include "edits/ui_logon_hours_dialog.h"

#include "adldap.h"
#include "edits/expiry_widget.h"
#include "globals.h"

#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QDebug>
#include <QDateTime>
#include <QTimeZone>
#include <QRadioButton>
#include <QPushButton>

LogonHoursEdit::LogonHoursEdit(QPushButton *button_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    button = button_arg;
    dialog = new LogonHoursDialog(button);

    connect(
        button, &QPushButton::clicked,
        dialog, &QDialog::open);
    connect(
        dialog, &LogonHoursDialog::accepted,
        [this]() {
            emit edited();
        });
}

void LogonHoursEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const QByteArray value = object.get_value(ATTRIBUTE_LOGON_HOURS);
    dialog->load(value);
}

void LogonHoursEdit::set_read_only(const bool read_only) {
    button->setEnabled(read_only);
}

bool LogonHoursEdit::apply(AdInterface &ad, const QString &dn) const {
    const QByteArray new_value = dialog->get();
    const bool success = ad.attribute_replace_value(dn, ATTRIBUTE_LOGON_HOURS, new_value);

    return success;
}

QList<bool> shift_list(const QList<bool> &list, const int shift_amount);

LogonHoursDialog::LogonHoursDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::LogonHoursDialog();
    ui->setupUi(this);

    model = new QStandardItemModel(DAYS_IN_WEEK, HOURS_IN_DAY, this);
    model->setVerticalHeaderLabels({
        tr("Sunday"),
        tr("Monday"),
        tr("Tuesday"),
        tr("Wednesday"),
        tr("Thursday"),
        tr("Friday"),
        tr("Saturday"),
    });

    ui->view->setModel(model);
    ui->view->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    for (int col = 0; col < model->columnCount(); col++) {
        ui->view->setColumnWidth(col, 5);
    }

    ui->local_time_button->setChecked(true);
    is_local_time = true;

    connect(
        ui->local_time_button, &QRadioButton::toggled,
        this, &LogonHoursDialog::on_local_time_button_toggled);
}

void LogonHoursDialog::load(const QByteArray &value) {
    original_value = value;

    ui->view->clearSelection();

    // NOTE: value may be empty if it's undefined
    if (value.size() != LOGON_HOURS_SIZE) {
        return;
    }

    const QList<QList<bool>> bools = logon_hours_to_bools(value, get_offset());

    for (int day = 0; day < DAYS_IN_WEEK; day++) {
        for (int h = 0; h < HOURS_IN_DAY; h++) {
            const bool selected = bools[day][h];

            if (selected) {
                const int row = day;
                const int column = h;
                const QModelIndex index = model->index(row, column);
                ui->view->selectionModel()->select(index, QItemSelectionModel::Select);
            }
        }
    }
}

QByteArray LogonHoursDialog::get() const {
    const QList<QList<bool>> bools = [&]() {
        QList<QList<bool>> out = logon_hours_to_bools(QByteArray(LOGON_HOURS_SIZE, '\0'));

        const QList<QModelIndex> selected = ui->view->selectionModel()->selectedIndexes();

        for (const QModelIndex &index : selected) {
            const int day = index.row();
            const int h = index.column();

            out[day][h] = true;
        }

        return out;
    }();

    const QByteArray out = logon_hours_to_bytes(bools, get_offset());

    return out;
}

void LogonHoursDialog::accept() {


    QDialog::accept();
}

void LogonHoursDialog::reject() {
    load(original_value);

    QDialog::reject();
}

// Get current value, change time state and reload value
void LogonHoursDialog::on_local_time_button_toggled(bool checked) {
    const QByteArray current_value = get();

    // NOTE: important to change state after get() call so
    // current_value is in correct format
    is_local_time = checked;

    load(current_value);
}

int get_current_utc_offset() {
    const QDateTime current_datetime = QDateTime::currentDateTime();
    const int offset_s = QTimeZone::systemTimeZone().offsetFromUtc(current_datetime);
    const int offset_h = offset_s / 60 / 60;

    return offset_h;
}

int LogonHoursDialog::get_offset() const {
    if (is_local_time) {
        return get_current_utc_offset();
    } else {
        return 0;
    }
}

QList<QList<bool>> logon_hours_to_bools(const QByteArray &byte_list, const int time_offset) {
    // Convet byte array to list of bools
    const QList<bool> joined = [&]() {
        QList<bool> out;

        for (const char byte : byte_list) {
            for (int bit_i = 0; bit_i < 8; bit_i++) {
                const int bit = (0x01 << bit_i);
                const bool is_set = bit_is_set((int) byte, bit);
                out.append(is_set);
            }
        }

        out = shift_list(out, time_offset);

        return out;
    }();

    // Split the list into sublists for each day
    const QList<QList<bool>> out = [&]() {
        QList<QList<bool>> out_the;

        for (int i = 0; i < joined.size(); i += HOURS_IN_DAY) {
            const QList<bool> day_list = joined.mid(i, HOURS_IN_DAY);
            out_the.append(day_list);
        }

        return out_the;        
    }();

    return out;
}

QByteArray logon_hours_to_bytes(const QList<QList<bool>> bool_list, const int time_offset) {
    const QList<bool> joined = [&]() {
        QList<bool> out;

        for (const QList<bool> &sublist : bool_list) {
            out += sublist;
        }

        out = shift_list(out, -time_offset);

        return out;
    }();

    const QByteArray out = [&]() {
        QByteArray bytes;

        for (int i = 0; i * 8 < joined.size(); i++) {
            const QList<bool> byte_list = joined.mid(i * 8, 8);

            int byte = 0;
            for (int bit_i = 0; bit_i < 8; bit_i++) {
                const int bit = (0x01 << bit_i);
                byte = bit_set(byte, bit, byte_list[bit_i]);
            }

            bytes.append(byte);
        }

        return bytes;
    }();

    return out;
}

QList<bool> shift_list(const QList<bool> &list, const int shift_amount) {
    if (abs(shift_amount) > list.size()) {
        return list;
    }

    QList<bool> out;

    for (int i = 0; i < list.size(); i++) {
        const int shifted_i = [&]() {
            int out_i = i - shift_amount;

            if (out_i < 0) {
                out_i += list.size();
            } else if (out_i >= list.size()) {
                out_i -= list.size();
            }

            return out_i;
        }();

        out.append(list[shifted_i]);
    }

    return out;
}
