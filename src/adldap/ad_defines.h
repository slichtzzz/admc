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

#ifndef AD_DEFINES_H
#define AD_DEFINES_H

/**
 * AD related enums and constants.
 */

enum AttributeType {
    AttributeType_Boolean,
    AttributeType_Enumeration,
    AttributeType_Integer,
    AttributeType_LargeInteger,
    AttributeType_StringCase,
    AttributeType_IA5,
    AttributeType_NTSecDesc,
    AttributeType_Numeric,
    AttributeType_ObjectIdentifier,
    AttributeType_Octet,
    AttributeType_ReplicaLink,
    AttributeType_Printable,
    AttributeType_Sid,
    AttributeType_Teletex,
    AttributeType_Unicode,
    AttributeType_UTCTime,
    AttributeType_GeneralizedTime,
    AttributeType_DNString,
    AttributeType_DNBinary,
    AttributeType_DSDN,
};

// NOTE: large integer type has sub types but AD schema
// doesn't distinguish between them (from what I've seen).
// Create enums for subtypes for easier processing.
enum LargeIntegerSubtype {
    LargeIntegerSubtype_Integer,
    LargeIntegerSubtype_Datetime,
    LargeIntegerSubtype_Timespan,
};

enum AccountOption {
    AccountOption_Disabled,
    AccountOption_CantChangePassword,
    AccountOption_PasswordExpired,
    AccountOption_DontExpirePassword,
    AccountOption_UseDesKey,
    AccountOption_SmartcardRequired,
    AccountOption_CantDelegate,
    AccountOption_DontRequirePreauth,
    AccountOption_TrustedForDelegation,
    AccountOption_COUNT
};

enum GroupScope {
    GroupScope_Global,
    GroupScope_DomainLocal,
    GroupScope_Universal,
    GroupScope_COUNT
};

enum GroupType {
    GroupType_Security,
    GroupType_Distribution,
    GroupType_COUNT
};

enum SystemFlagsBit {
    SystemFlagsBit_CannotMove = 0x04000000,
    SystemFlagsBit_CannotRename = 0x08000000,
    SystemFlagsBit_CannotDelete = 0x80000000
};

#define ATTRIBUTE_CN "cn"
#define ATTRIBUTE_USER_ACCOUNT_CONTROL "userAccountControl"
#define ATTRIBUTE_LOCKOUT_TIME "lockoutTime"
#define ATTRIBUTE_ACCOUNT_EXPIRES "accountExpires"
#define ATTRIBUTE_PWD_LAST_SET "pwdLastSet"
#define ATTRIBUTE_NAME "name"
#define ATTRIBUTE_INITIALS "initials"
#define ATTRIBUTE_SAMACCOUNT_NAME "sAMAccountName"
#define ATTRIBUTE_DISPLAY_NAME "displayName"
#define ATTRIBUTE_DESCRIPTION "description"
#define ATTRIBUTE_USER_PRINCIPAL_NAME "userPrincipalName"
#define ATTRIBUTE_MAIL "mail"
#define ATTRIBUTE_OFFICE "physicalDeliveryOfficeName"
#define ATTRIBUTE_TELEPHONE_NUMBER "telephoneNumber"
#define ATTRIBUTE_TELEPHONE_NUMBER_OTHER "otherTelephone"
#define ATTRIBUTE_WWW_HOMEPAGE "wWWHomePage"
#define ATTRIBUTE_WWW_HOMEPAGE_OTHER "url"
#define ATTRIBUTE_COUNTRY_ABBREVIATION "c"
#define ATTRIBUTE_COUNTRY "co"
#define ATTRIBUTE_COUNTRY_CODE "countryCode"
#define ATTRIBUTE_CITY "l"
#define ATTRIBUTE_PO_BOX "postOfficeBox"
#define ATTRIBUTE_POSTAL_CODE "postalCode"
#define ATTRIBUTE_STATE "st"
#define ATTRIBUTE_STREET "streetAddress"
#define ATTRIBUTE_DN "distinguishedName"
#define ATTRIBUTE_OBJECT_CLASS "objectClass"
#define ATTRIBUTE_WHEN_CREATED "whenCreated"
#define ATTRIBUTE_WHEN_CHANGED "whenChanged"
#define ATTRIBUTE_USN_CHANGED "uSNChanged"
#define ATTRIBUTE_USN_CREATED "uSNCreated"
#define ATTRIBUTE_OBJECT_CATEGORY "objectCategory"
#define ATTRIBUTE_MEMBER "member"
#define ATTRIBUTE_MEMBER_OF "memberOf"
#define ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY "showInAdvancedViewOnly"
#define ATTRIBUTE_GROUP_TYPE "groupType"
#define ATTRIBUTE_FIRST_NAME "givenName"
#define ATTRIBUTE_LAST_NAME "sn"
#define ATTRIBUTE_DNS_HOST_NAME "dNSHostName"
#define ATTRIBUTE_INFO "info"
#define ATTRIBUTE_PASSWORD "unicodePwd"
#define ATTRIBUTE_GPLINK "gPLink"
#define ATTRIBUTE_GPOPTIONS "gPOptions"
#define ATTRIBUTE_DEPARTMENT "department"
#define ATTRIBUTE_COMPANY "company"
#define ATTRIBUTE_TITLE "title"
#define ATTRIBUTE_LAST_LOGON "lastLogon"
#define ATTRIBUTE_LAST_LOGON_TIMESTAMP "lastLogonTimestamp"
#define ATTRIBUTE_PWD_LAST_SET "pwdLastSet"
#define ATTRIBUTE_LOCKOUT_TIME "lockoutTime"
#define ATTRIBUTE_BAD_PWD_TIME "badPasswordTime"
#define ATTRIBUTE_OBJECT_SID "objectSid"
#define ATTRIBUTE_SYSTEM_FLAGS "systemFlags"
#define ATTRIBUTE_MAX_PWD_AGE "maxPwdAge"
#define ATTRIBUTE_MIN_PWD_AGE "minPwdAge"
#define ATTRIBUTE_LOCKOUT_DURATION "lockoutDuration"
#define ATTRIBUTE_IS_CRITICAL_SYSTEM_OBJECT "isCriticalSystemObject"
#define ATTRIBUTE_GPC_FILE_SYS_PATH "gPCFileSysPath"
#define ATTRIBUTE_GPC_FUNCTIONALITY_VERSION "gpCFunctionalityVersion"
#define ATTRIBUTE_VERSION_NUMBER "versionNumber"
#define ATTRIBUTE_FLAGS "flags"
#define ATTRIBUTE_OBJECT_GUID "objectGUID"
#define ATTRIBUTE_PRIMARY_GROUP_ID "primaryGroupID"
#define ATTRIBUTE_MANAGER "manager"
#define ATTRIBUTE_MANAGED_BY "managedBy"
#define ATTRIBUTE_DIRECT_REPORTS "directReports"
#define ATTRIBUTE_PROFILE_PATH "profilePath"
#define ATTRIBUTE_SCRIPT_PATH "scriptPath"
#define ATTRIBUTE_HOME_DIRECTORY "homeDirectory"

#define ATTRIBUTE_HOME_PHONE "homePhone"
#define ATTRIBUTE_OTHER_HOME_PHONE "otherHomePhone"
#define ATTRIBUTE_PAGER "pager"
#define ATTRIBUTE_OTHER_PAGER "otherPager"
#define ATTRIBUTE_MOBILE "mobile"
#define ATTRIBUTE_OTHER_MOBILE "otherMobile"
#define ATTRIBUTE_FAX_NUMBER "facsimileTelephoneNumber"
#define ATTRIBUTE_OTHER_FAX_NUMBER "otherFacsimileTelephoneNumber"
#define ATTRIBUTE_IP_PHONE "ipPhone"
#define ATTRIBUTE_OTHER_IP_PHONE "otherIpPhone"
#define ATTRIBUTE_UPN_SUFFIXES "uPNSuffixes"
#define ATTRIBUTE_SECURITY_DESCRIPTOR "nTSecurityDescriptor"
#define ATTRIBUTE_RIGHTS_GUID "rightsGuid"
#define ATTRIBUTE_LOCATION "location"
#define ATTRIBUTE_OS "operatingSystem"
#define ATTRIBUTE_OS_VERSION "operatingSystemVersion"
#define ATTRIBUTE_OS_SERVICE_PACK "operatingSystemServicePack"
#define ATTRIBUTE_LOGON_HOURS "logonHours"
#define ATTRIBUTE_USER_WORKSTATIONS "userWorkstations"

#define CLASS_GROUP "group"
#define CLASS_USER "user"
#define CLASS_CONTAINER "container"
#define CLASS_OU "organizationalUnit"
#define CLASS_PERSON "person"
#define CLASS_GP_CONTAINER "groupPolicyContainer"
#define CLASS_DOMAIN "domainDNS"
#define CLASS_TOP "top"
#define CLASS_COMPUTER "computer"
#define CLASS_ORG_PERSON "organizationalPerson"
#define CLASS_CONTACT "contact"
// TODO: not sure if this is correct class for printer, but display name for this in display specifiers is "Printer"
#define CLASS_PRINTER "printQueue"
#define CLASS_DEFAULT "default"
#define CLASS_CONFIGURATION "configuration"
#define CLASS_TRUSTED_DOMAIN "trustedDomain"
#define CLASS_INET_ORG_PERSON "inetOrgPerson"
#define CLASS_FOREIGN_SECURITY_PRINCIPAL "foreignSecurityPrincipal"
#define CLASS_SHARED_FOLDER "volume"
#define CLASS_RPC_SERVICES "rpcContainer"
#define CLASS_CERTIFICATE_TEMPLATE "pKICertificateTemplate"
#define CLASS_MSMQ_GROUP "msMQ-Group"
#define CLASS_MSMQ_QUEUE_ALIAS "msMQ-Custom-Recipient"
#define CLASS_REMOTE_STORAGE_SERVICE "remoteStorageServicePoint"
// NOTE: for schema object
#define CLASS_dMD "dMD"
#define CLASS_CONTROL_ACCESS_RIGHT "controlAccessRight"

#define LOCKOUT_UNLOCKED_VALUE "0"

#define AD_LARGE_INTEGER_DATETIME_NEVER_1 "0"
#define AD_LARGE_INTEGER_DATETIME_NEVER_2 "9223372036854775807"

#define AD_PWD_LAST_SET_EXPIRED "0"
#define AD_PWD_LAST_SET_RESET "-1"

#define LDAP_BOOL_TRUE "TRUE"
#define LDAP_BOOL_FALSE "FALSE"

#define GPOPTIONS_INHERIT "0"
#define GPOPTIONS_BLOCK_INHERITANCE "1"

#define GROUP_TYPE_BIT_SECURITY 0x80000000
#define GROUP_TYPE_BIT_SYSTEM 0x00000001

#define DATETIME_DISPLAY_FORMAT "dd.MM.yy hh:mm UTCt"

const long long MILLIS_TO_100_NANOS = 10000LL;

#define LDAP_SERVER_SD_FLAGS_OID "1.2.840.113556.1.4.801"
#define OWNER_SECURITY_INFORMATION 0x01
#define GROUP_SECURITY_INFORMATION 0x04
#define SACL_SECURITY_INFORMATION 0x08
#define DACL_SECURITY_INFORMATION 0x10

enum PermissionState {
    PermissionState_None,
    PermissionState_Allowed,
    PermissionState_Denied,
};

enum AcePermission {
    AcePermission_FullControl,
    AcePermission_Read,
    AcePermission_Write,
    AcePermission_Delete,
    AcePermission_DeleteSubtree,
    AcePermission_CreateChild,
    AcePermission_DeleteChild,
    AcePermission_AllowedToAuthenticate,
    AcePermission_ChangePassword,
    AcePermission_ReceiveAs,
    AcePermission_ResetPassword,
    AcePermission_SendAs,
    AcePermission_ReadAccountRestrictions,
    AcePermission_WriteAccountRestrictions,
    AcePermission_ReadGeneralInfo,
    AcePermission_WriteGeneralInfo,
    AcePermission_ReadGroupMembership,
    AcePermission_ReadLogonInfo,
    AcePermission_WriteLogonInfo,
    AcePermission_ReadPersonalInfo,
    AcePermission_WritePersonalInfo,
    AcePermission_ReadPhoneAndMailOptions,
    AcePermission_WritePhoneAndMailOptions,
    AcePermission_ReadPrivateInfo,
    AcePermission_WritePrivateInfo,
    AcePermission_ReadPublicInfo,
    AcePermission_WritePublicInfo,
    AcePermission_ReadRemoteAccessInfo,
    AcePermission_WriteRemoteAccessInfo,
    AcePermission_ReadTerminalServerLicenseServer,
    AcePermission_WriteTerminalServerLicenseServer,
    AcePermission_ReadWebInfo,
    AcePermission_WriteWebInfo,

    AcePermission_COUNT,
};

enum SearchScope {
    SearchScope_Object,
    SearchScope_Children,
    // NOTE: Descendants scope appears to not work, from the
    // ldap_search manual: "Note that the latter requires
    // the server support the LDAP Subordinates Search Scope
    // extension."
    SearchScope_Descendants,
    SearchScope_All,
};

enum CertStrategy {
    CertStrategy_Never,
    CertStrategy_Hard,
    CertStrategy_Demand,
    CertStrategy_Allow,
    CertStrategy_Try,
};

#endif /* AD_DEFINES_H */
