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

#ifndef RENAME_DIALOG_H
#define RENAME_DIALOG_H

#include <QDialog>

class QLineEdit;
class QDialogButtonBox;
class AttributeEdit;
class AdInterface;

class RenameDialog : public QDialog {
    Q_OBJECT

public:
    using QDialog::QDialog;

    static void success_msg(const QString &old_name);
    static void fail_msg(const QString &old_name);

    void init(QLineEdit *name_edit, QDialogButtonBox *button_box, const QList<AttributeEdit *> &edits);

    void set_target(const QString &dn);
    void reset();
    QString get_new_dn() const;
    void open() override;
    void accept() override;

private:
    QString target;
    QLineEdit *name_edit;
    QList<AttributeEdit *> edits;
};

#endif /* RENAME_DIALOG_H */
