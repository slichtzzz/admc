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

#include "admc_test_gplink.h"

#include "gplink.h"

Q_DECLARE_METATYPE(GplinkOption)

const QString test_gplink_string = "[LDAP://cn={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},cn=policies,cn=system,DC=domain,DC=alt;0][LDAP://cn={BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB},cn=policies,cn=system,DC=domain,DC=alt;1][LDAP://cn={CCCCCCCC-CCCC-CCCC-CCCC-CCCCCCCCCCCC},cn=policies,cn=system,DC=domain,DC=alt;2]";

const QString dn_A = "CN={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},CN=Policies,CN=System,DC=domain,DC=alt";
const QString dn_B = "CN={BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB},CN=Policies,CN=System,DC=domain,DC=alt";
const QString dn_C = "CN={CCCCCCCC-CCCC-CCCC-CCCC-CCCCCCCCCCCC},CN=Policies,CN=System,DC=domain,DC=alt";

const QString gplink_A = "[LDAP://cn={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},cn=policies,cn=system,DC=domain,DC=alt;0]";
const QString gplink_B = "[LDAP://cn={BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB},cn=policies,cn=system,DC=domain,DC=alt;1]";
const QString gplink_C = "[LDAP://cn={CCCCCCCC-CCCC-CCCC-CCCC-CCCCCCCCCCCC},cn=policies,cn=system,DC=domain,DC=alt;2]";

void test_gplink_equality(const Gplink &a, const QString &b);

void ADMCTestGplink::initTestCase() {
}

void ADMCTestGplink::cleanupTestCase() {
}

void ADMCTestGplink::init() {
}

void ADMCTestGplink::cleanup() {
}

void ADMCTestGplink::to_string() {
    Gplink gplink(test_gplink_string);

    QCOMPARE(gplink.to_string(), test_gplink_string);
}

void ADMCTestGplink::equals() {
    Gplink gplink_1(test_gplink_string);
    Gplink gplink_2(test_gplink_string);

    QCOMPARE(gplink_1.to_string(), gplink_2.to_string());
}

void ADMCTestGplink::contains_data() {
    QTest::addColumn<QString>("gpo");
    QTest::addColumn<bool>("contains_value");

    QTest::newRow("a") << dn_A << true;
    QTest::newRow("b") << dn_B << true;
    QTest::newRow("c") << dn_C << true;
    QTest::newRow("shouldn't contains") << "CN={00000000-016D-11D2-945F-00C04FB984F9},CN=Policies,CN=Dystem,DC=domain,DC=alt" << false;
}

void ADMCTestGplink::contains() {
    QFETCH(QString, gpo);
    QFETCH(bool, contains_value);
    
    Gplink gplink(test_gplink_string);

    QCOMPARE(gplink.contains(gpo), contains_value);
}

void ADMCTestGplink::add_data() {
    QTest::addColumn<QString>("gplink_before");
    QTest::addColumn<QList<QString>>("gpo_list");
    QTest::addColumn<QString>("gplink_after");

    QTest::newRow("add to empty") << "" << QList<QString>({dn_A}) << gplink_A;
    QTest::newRow("add to non-empty") << gplink_B << QList<QString>({dn_A}) << (gplink_B + gplink_A);
    QTest::newRow("add same one twice to empty") << "" << QList<QString>({dn_A, dn_A}) << gplink_A;
    QTest::newRow("add already existing") << "gplink_A" << QList<QString>({dn_A}) << gplink_A;
}

void ADMCTestGplink::add() {
    QFETCH(QString, gplink_before);
    QFETCH(QList<QString>, gpo_list);
    QFETCH(QString, gplink_after);

    Gplink gplink(gplink_before);

    for (const QString &gpo : gpo_list) {
        gplink.add(gpo);
    }

    QCOMPARE(gplink.to_string(), gplink_after);
}

void ADMCTestGplink::remove_data() {
    QTest::addColumn<QString>("gplink_before");
    QTest::addColumn<QList<QString>>("gpo_list");
    QTest::addColumn<QString>("gplink_after");

    QTest::newRow("remove 1") << gplink_A << QList<QString>({dn_A}) << "";
    QTest::newRow("remove from empty") << "" << QList<QString>({dn_A}) << "";
    QTest::newRow("remove same one twice") << gplink_A << QList<QString>({dn_A, dn_A}) << "";
    QTest::newRow("remove from multiple") << (gplink_A + gplink_B) << QList<QString>({dn_A}) << gplink_B;
}

void ADMCTestGplink::remove() {
    QFETCH(QString, gplink_before);
    QFETCH(QList<QString>, gpo_list);
    QFETCH(QString, gplink_after);

    Gplink gplink(gplink_before);

    for (const QString &gpo : gpo_list) {
        gplink.remove(gpo);
    }

    QCOMPARE(gplink.to_string(), gplink_after);
}

void ADMCTestGplink::move_up_data() {
    QTest::addColumn<QString>("gplink_before");
    QTest::addColumn<QList<QString>>("gpo_list");
    QTest::addColumn<QString>("gplink_after");

    QTest::newRow("move up empty") << "" << QList<QString>({dn_A}) << "";
    QTest::newRow("move up single") << gplink_A << QList<QString>({dn_A}) << gplink_A;
    QTest::newRow("move up non-existing") << gplink_A << QList<QString>({dn_B}) << gplink_A;
    QTest::newRow("move up from [2] to [1]") << (gplink_A + gplink_B + gplink_C) << QList<QString>({dn_C}) << (gplink_A + gplink_C + gplink_B);
    QTest::newRow("move up twice from [2] to [0]") << (gplink_A + gplink_B + gplink_C) << QList<QString>({dn_C, dn_C}) << (gplink_C + gplink_A + gplink_B);
}

void ADMCTestGplink::move_up() {
    QFETCH(QString, gplink_before);
    QFETCH(QList<QString>, gpo_list);
    QFETCH(QString, gplink_after);

    Gplink gplink(gplink_before);

    for (const QString &gpo : gpo_list) {
        gplink.move_up(gpo);
    }

    QCOMPARE(gplink.to_string(), gplink_after);
}

void ADMCTestGplink::move_down_data() {
    QTest::addColumn<QString>("gplink_before");
    QTest::addColumn<QList<QString>>("gpo_list");
    QTest::addColumn<QString>("gplink_after");

    QTest::newRow("move down empty") << "" << QList<QString>({dn_A}) << "";
    QTest::newRow("move down single") << gplink_A << QList<QString>({dn_A}) << gplink_A;
    QTest::newRow("move down non-existing") << gplink_A << QList<QString>({dn_B}) << gplink_A;
    QTest::newRow("move down from [1] to [2]") << (gplink_A + gplink_B + gplink_C) << QList<QString>({dn_B}) << (gplink_A + gplink_C + gplink_B);
    QTest::newRow("move down twice from [0] to [2]") << (gplink_A + gplink_B + gplink_C) << QList<QString>({dn_A, dn_A}) << (gplink_B + gplink_C + gplink_A);
}

void ADMCTestGplink::move_down() {
    QFETCH(QString, gplink_before);
    QFETCH(QList<QString>, gpo_list);
    QFETCH(QString, gplink_after);

    Gplink gplink(gplink_before);

    for (const QString &gpo : gpo_list) {
        gplink.move_down(gpo);
    }

    QCOMPARE(gplink.to_string(), gplink_after);
}

void ADMCTestGplink::get_option_data() {
    QTest::addColumn<QString>("gplink_before");
    QTest::addColumn<QString>("gpo");
    QTest::addColumn<GplinkOption>("option");
    QTest::addColumn<bool>("option_value");

    QTest::newRow("A disabled") << gplink_A << dn_A << GplinkOption_Disabled << false;
    QTest::newRow("A enforced") << gplink_A << dn_A << GplinkOption_Enforced << false;

    QTest::newRow("B disabled") << gplink_B << dn_B << GplinkOption_Disabled << true;
    QTest::newRow("B enforced") << gplink_B << dn_B << GplinkOption_Enforced << false;

    QTest::newRow("C disabled") << gplink_C << dn_C << GplinkOption_Disabled << false;
    QTest::newRow("C enforced") << gplink_C << dn_C << GplinkOption_Enforced << true;
}

void ADMCTestGplink::get_option() {
    QFETCH(QString, gplink_before);
    QFETCH(QString, gpo);
    QFETCH(GplinkOption, option);
    QFETCH(bool, option_value);

    Gplink gplink(gplink_before);

    const bool actual_value = gplink.get_option(gpo, option);

    QCOMPARE(actual_value, option_value);
}

void ADMCTestGplink::set_option_data() {
    QTest::addColumn<QString>("gplink_before");
    QTest::addColumn<QString>("gpo");
    QTest::addColumn<GplinkOption>("option");
    QTest::addColumn<bool>("option_value");
    QTest::addColumn<QString>("gplink_after");

    QTest::newRow("set disabled true") << "[LDAP://cn={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},cn=policies,cn=system,DC=domain,DC=alt;0]" << dn_A << GplinkOption_Disabled << true << "[LDAP://cn={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},cn=policies,cn=system,DC=domain,DC=alt;1]";
    QTest::newRow("set enforced true") << "[LDAP://cn={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},cn=policies,cn=system,DC=domain,DC=alt;0]" << dn_A << GplinkOption_Enforced << true << "[LDAP://cn={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},cn=policies,cn=system,DC=domain,DC=alt;2]";
    QTest::newRow("set disabled false") << "[LDAP://cn={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},cn=policies,cn=system,DC=domain,DC=alt;1]" << dn_A << GplinkOption_Disabled << false << "[LDAP://cn={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},cn=policies,cn=system,DC=domain,DC=alt;0]";
    QTest::newRow("set enforced false") << "[LDAP://cn={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},cn=policies,cn=system,DC=domain,DC=alt;2]" << dn_A << GplinkOption_Enforced << false << "[LDAP://cn={AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA},cn=policies,cn=system,DC=domain,DC=alt;0]";
}

void ADMCTestGplink::set_option() {
    QFETCH(QString, gplink_before);
    QFETCH(QString, gpo);
    QFETCH(GplinkOption, option);
    QFETCH(bool, option_value);
    QFETCH(QString, gplink_after);

    Gplink gplink(gplink_before);

    gplink.set_option(gpo, option, option_value);

    QCOMPARE(gplink.to_string(), gplink_after);
}

QTEST_MAIN(ADMCTestGplink)
