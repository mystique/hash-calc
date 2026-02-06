#ifndef RESOURCE_H
#define RESOURCE_H

// Dialog ID
#define IDD_MAIN_DIALOG 101
#define IDI_APP_ICON 102

// Tab View Dialog IDs
#define IDD_TAB_VIEW_SHA 201
#define IDD_TAB_VIEW_SHA3 202
#define IDD_TAB_VIEW_HAVAL 203
#define IDD_TAB_VIEW_CHECKSUM 204

// ========== TAB 1: SHA & MD ==========
// SHA Group (160-512)
#define IDC_SHA_160 1001
#define IDC_SHA_224 1002
#define IDC_SHA_256 1003
#define IDC_SHA_384 1004
#define IDC_SHA_512 1005

// MD Group (MD2, MD4, MD5)
#define IDC_MD2 1011
#define IDC_MD4 1012
#define IDC_MD5 1013

// MD6 Group (128-512)
#define IDC_MD6_128 1021
#define IDC_MD6_160 1022
#define IDC_MD6_192 1023
#define IDC_MD6_224 1024
#define IDC_MD6_256 1025
#define IDC_MD6_384 1026
#define IDC_MD6_512 1027

// ========== TAB 2: SHA-3 & Modern ==========
// SHA-3 Group (224-512)
#define IDC_SHA3_224 2001
#define IDC_SHA3_256 2002
#define IDC_SHA3_384 2003
#define IDC_SHA3_512 2004

// Keccak Group (224-512)
#define IDC_KECCAK_224 2011
#define IDC_KECCAK_256 2012
#define IDC_KECCAK_384 2013
#define IDC_KECCAK_512 2014

// SHAKE Group (128, 256)
#define IDC_SHAKE_128 2021
#define IDC_SHAKE_256 2022

// BLAKE Group (BLAKE2b, BLAKE2s)
#define IDC_BLAKE2B 2031
#define IDC_BLAKE2S 2032

// Asian Group (LSH-256, SM3, LSH-512)
#define IDC_LSH_256 2041
#define IDC_SM3 2042
#define IDC_LSH_512 2043

// ========== TAB 3: HAVAL & RIPEMD ==========
// HAVAL Group (128-256)
#define IDC_HAVAL_128 3001
#define IDC_HAVAL_160 3002
#define IDC_HAVAL_192 3003
#define IDC_HAVAL_224 3004
#define IDC_HAVAL_256 3005

// HAVAL Pass Selections
#define IDC_HAVAL_PASS3 3011
#define IDC_HAVAL_PASS4 3012
#define IDC_HAVAL_PASS5 3013
#define IDC_STATIC_HAVAL_PASS 3014

// RIPEMD Group (128-320)
#define IDC_RIPEMD_128 3021
#define IDC_RIPEMD_160 3022
#define IDC_RIPEMD_256 3023
#define IDC_RIPEMD_320 3024

// ========== TAB 4: Checksum & Others ==========
// Checksum Group
#define IDC_CRC32 4001
#define IDC_ADLER32 4002
#define IDC_CRC16 4003
#define IDC_CRC32C 4004
#define IDC_CRC64 4005
#define IDC_CRC8 4006

// Others Group (Tiger, Whirlpool)
#define IDC_TIGER 4011
#define IDC_WHIRLPOOL 4012

// ========== UI Elements ==========
// Select/Clear buttons
#define IDC_SELECT_ALL 5001
#define IDC_CLEAR_ALL 5002

// Input type radio buttons
#define IDC_RADIO_TEXT 5011
#define IDC_RADIO_FILE 5012

// Input controls
#define IDC_EDIT_TEXT 5021
#define IDC_EDIT_FILE 5022
#define IDC_BUTTON_BROWSE 5023

// Result display
#define IDC_EDIT_RESULT 5031

// Bottom buttons
#define IDC_CHECK_STAY_ON_TOP 5041
#define IDC_BUTTON_CALCULATE 5042
#define IDC_BUTTON_ABOUT 5043
#define IDC_BUTTON_EXIT 5044
#define IDC_PROGRESS_CALC 5045

// Tab Control
#define IDC_TAB_MAIN 5051

// ========== Group Boxes ==========
// Tab 1 Groups
#define IDC_GROUP_SHA 6001
#define IDC_GROUP_MD 6002
#define IDC_GROUP_MD6 6003

// Tab 2 Groups
#define IDC_GROUP_SHA3 6011
#define IDC_GROUP_KECCAK 6012
#define IDC_GROUP_SHAKE 6013
#define IDC_GROUP_BLAKE 6014
#define IDC_GROUP_LSH 6015

// Tab 3 Groups
#define IDC_GROUP_HAVAL 6021
#define IDC_GROUP_RIPEMD 6022

// Tab 4 Groups
#define IDC_GROUP_CHECKSUM 6031
#define IDC_GROUP_MISC 6032

// Input Group
#define IDC_GROUP_INPUT 6041

// ========== System Tray ==========
// Tray icon message ID
#define WM_TRAYICON (WM_USER + 100)

// Tray menu items
#define IDM_TRAY_RESTORE 7001
#define IDM_TRAY_STOP 7002
#define IDM_TRAY_EXIT 7003

#endif // RESOURCE_H
