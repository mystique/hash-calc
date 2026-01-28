#ifndef RESOURCE_H
#define RESOURCE_H

// Dialog ID
#define IDD_MAIN_DIALOG 101
#define IDI_APP_ICON 102

// SHA Checkboxes
#define IDC_SHA_160 1001
#define IDC_SHA_224 1002
#define IDC_SHA_256 1003
#define IDC_SHA_384 1004
#define IDC_SHA_512 1005

// HAVAL Checkboxes
#define IDC_HAVAL_128 1010
#define IDC_HAVAL_160 1011
#define IDC_HAVAL_192 1012
#define IDC_HAVAL_224 1013
#define IDC_HAVAL_256 1014

// HAVAL passes
#define IDC_HAVAL_PASS3 1020
#define IDC_HAVAL_PASS4 1021
#define IDC_HAVAL_PASS4 1021
#define IDC_HAVAL_PASS5 1022
#define IDC_STATIC_HAVAL_PASS 1023

// Others Checkboxes
#define IDC_RIPEMD_160 1030
#define IDC_MD5 1031
#define IDC_CRC32 1032

// Select/Clear buttons
#define IDC_SELECT_ALL 1040
#define IDC_CLEAR_ALL 1041

// Input type radio buttons
#define IDC_RADIO_TEXT 1050
#define IDC_RADIO_FILE 1051

// Input controls
#define IDC_EDIT_TEXT 1060
#define IDC_EDIT_FILE 1061
#define IDC_BUTTON_BROWSE 1062

// Result display
#define IDC_EDIT_RESULT 1070

// Bottom buttons
#define IDC_CHECK_STAY_ON_TOP 1080
#define IDC_BUTTON_CALCULATE 1081
#define IDC_BUTTON_ABOUT 1082
#define IDC_BUTTON_EXIT 1083

// Group boxes
#define IDC_GROUP_SHA 1090
#define IDC_GROUP_HAVAL 1091
#define IDC_GROUP_OTHERS 1092
#define IDC_GROUP_INPUT 1093

// New UI Elements
#define IDC_TAB_MAIN 1100

// SHA-3 Group
#define IDC_GROUP_SHA3 1110
#define IDC_SHA3_224 1111
#define IDC_SHA3_256 1112
#define IDC_SHA3_384 1113
#define IDC_SHA3_512 1114

// RIPEMD Group
#define IDC_GROUP_RIPEMD 1120
#define IDC_RIPEMD_128 1121
// IDC_RIPEMD_160 is 1030
#define IDC_RIPEMD_256 1122
#define IDC_RIPEMD_320 1123

// Misc Group (Tiger, SM3, Whirlpool)
#define IDC_GROUP_MISC 1130
#define IDC_TIGER 1131
#define IDC_SM3 1132
#define IDC_WHIRLPOOL 1133

// MD Group
#define IDC_GROUP_MD 1200
#define IDC_MD2 1201
#define IDC_MD4 1202
// IDC_MD5 is 1031

// Checksum Group (Renamed from Others)
#define IDC_GROUP_CHECKSUM 1210
#define IDC_ADLER32 1213
// IDC_CRC32 is 1032

// BLAKE Group
#define IDC_GROUP_BLAKE 1220
#define IDC_BLAKE2B 1221
#define IDC_BLAKE2S 1222

// LSH Group
#define IDC_GROUP_LSH 1230
#define IDC_LSH_256 1231
#define IDC_LSH_512 1232

// SHA-3 Extensions
// SHA-3 Extensions
#define IDC_KECCAK_224 1240
#define IDC_KECCAK_256 1243
#define IDC_KECCAK_384 1244
#define IDC_KECCAK_512 1245
#define IDC_SHAKE_128 1241
#define IDC_SHAKE_256 1242

#define IDC_GROUP_KECCAK 1250
#define IDC_GROUP_SHAKE 1251

#endif // RESOURCE_H
