/*****************************************************************************
 *   Ledger Stellar App.
 *   (c) 2024 Ledger SAS.
 *   (c) 2024 overcat.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/
#ifdef HAVE_NBGL
#include "display.h"

#include "os.h"
#include "os_io_seproxyhal.h"
#include "glyphs.h"
#include "ux.h"
#include "nbgl_use_case.h"

#include "settings.h"
#include "globals.h"

//  -----------------------------------------------------------
//  ----------------------- HOME PAGE -------------------------
//  -----------------------------------------------------------
void app_quit(void) {
    // exit app here
    os_sched_exit(-1);
}

//  -----------------------------------------------------------
//  --------------------- SETTINGS MENU -----------------------
//  -----------------------------------------------------------
#define SETTING_INFO_NB 3
static const char* const INFO_TYPES[SETTING_INFO_NB] = {"Version", "Developer"};
static const char* const INFO_CONTENTS[SETTING_INFO_NB] = {APPVERSION, "overcat"};

// settings switches definitions
enum {
    SWITCH_UNVERIFIED_CONTRACTS_SET_TOKEN = FIRST_USER_TOKEN,
    SWITCH_HASH_SET_TOKEN,
    SWITCH_SEQUENCE_SET_TOKEN,
};

enum {
    SWITCH_UNVERIFIED_CONTRACTS_SET_ID = 0,
    SWITCH_HASH_SET_ID,
    SWITCH_SEQUENCE_SET_ID,
    SETTINGS_SWITCHES_NB
};

static nbgl_contentSwitch_t switches[SETTINGS_SWITCHES_NB] = {0};

static const nbgl_contentInfoList_t info_list = {
    .nbInfos = SETTING_INFO_NB,
    .infoTypes = INFO_TYPES,
    .infoContents = INFO_CONTENTS,
};

static uint8_t init_setting_page;
static void review_warning_choice_hash_signing(bool confirm);
static void review_warning_choice_unverified_contracts(bool confirm);
static void controls_callback(int token, uint8_t index, int page);

// settings menu definition
#define SETTING_CONTENTS_NB 1
static const nbgl_content_t contents[SETTING_CONTENTS_NB] = {
    {.type = SWITCHES_LIST,
     .content.switchesList.nbSwitches = SETTINGS_SWITCHES_NB,
     .content.switchesList.switches = switches,
     .contentActionCallback = controls_callback}};

static const nbgl_genericContents_t setting_contents = {.callbackCallNeeded = false,
                                                        .contentsList = contents,
                                                        .nbContents = SETTING_CONTENTS_NB};

// callback for setting warning choice
static void review_warning_choice_hash_signing(bool confirm) {
    if (confirm) {
        switches[SWITCH_HASH_SET_ID].initState = ON_STATE;
        // store the new setting value in NVM
        SETTING_TOGGLE(S_HASH_SIGNING_ENABLED);
    }

    // Reset setting menu to the right page
    nbgl_useCaseHomeAndSettings(APPNAME,
                                &C_icon_stellar_64px,
                                NULL,
                                init_setting_page,
                                &setting_contents,
                                &info_list,
                                NULL,
                                app_quit);
}

// callback for setting warning choice
static void review_warning_choice_unverified_contracts(bool confirm) {
    if (confirm) {
        switches[SWITCH_UNVERIFIED_CONTRACTS_SET_ID].initState = ON_STATE;
        // store the new setting value in NVM
        SETTING_TOGGLE(S_UNVERIFIED_CONTRACTS_ENABLED);
    }

    // Reset setting menu to the right page
    nbgl_useCaseHomeAndSettings(APPNAME,
                                &C_icon_stellar_64px,
                                NULL,
                                init_setting_page,
                                &setting_contents,
                                &info_list,
                                NULL,
                                app_quit);
}

static void controls_callback(int token, uint8_t index, int page) {
    UNUSED(index);
    init_setting_page = page;

    if (token == SWITCH_HASH_SET_TOKEN) {
        if (HAS_SETTING(S_HASH_SIGNING_ENABLED)) {
            switches[SWITCH_HASH_SET_ID].initState = OFF_STATE;
            SETTING_TOGGLE(S_HASH_SIGNING_ENABLED);
        } else {
            // Display the warning message and ask the user to confirm
            nbgl_useCaseChoice(
                &C_Warning_64px,
                "Enable Hash Signing",
                "Signing hashes is a dangerous operation that can put your wallet at serious risk. "
                "Only enable this feature if you are sure you know what you are doing.",
                "I understand, confirm",
                "Cancel",
                review_warning_choice_hash_signing);
        }
    } else if (token == SWITCH_SEQUENCE_SET_TOKEN) {
        // toggle the switch value
        switches[SWITCH_SEQUENCE_SET_ID].initState =
            (!HAS_SETTING(S_SEQUENCE_NUMBER_ENABLED)) ? ON_STATE : OFF_STATE;
        // store the new setting value in NVM
        SETTING_TOGGLE(S_SEQUENCE_NUMBER_ENABLED);
    } else if (token == SWITCH_UNVERIFIED_CONTRACTS_SET_TOKEN) {
        if (HAS_SETTING(S_UNVERIFIED_CONTRACTS_ENABLED)) {
            switches[SWITCH_UNVERIFIED_CONTRACTS_SET_ID].initState = OFF_STATE;
            SETTING_TOGGLE(S_UNVERIFIED_CONTRACTS_ENABLED);
        } else {
            // Display the warning message and ask the user to confirm
            nbgl_useCaseChoice(
                &C_Warning_64px,
                "Allow unverified contracts",
                "Unverified contracts may not be displayed in a readable form on "
                "your Ledger, so you need to examine them very carefully before sign them.",
                "I understand, confirm",
                "Cancel",
                review_warning_choice_unverified_contracts);
        }
    }
}

// home page definition
void ui_menu_main(void) {
    // Initialize switches data

    switches[SWITCH_UNVERIFIED_CONTRACTS_SET_ID].initState =
        (HAS_SETTING(S_UNVERIFIED_CONTRACTS_ENABLED)) ? ON_STATE : OFF_STATE;
    switches[SWITCH_UNVERIFIED_CONTRACTS_SET_ID].text = "Custom contracts";
    switches[SWITCH_UNVERIFIED_CONTRACTS_SET_ID].subText = "Allow unverified contract";
    switches[SWITCH_UNVERIFIED_CONTRACTS_SET_ID].token = SWITCH_UNVERIFIED_CONTRACTS_SET_TOKEN;
    switches[SWITCH_UNVERIFIED_CONTRACTS_SET_ID].tuneId = TUNE_TAP_CASUAL;

    switches[SWITCH_HASH_SET_ID].initState =
        (HAS_SETTING(S_HASH_SIGNING_ENABLED)) ? ON_STATE : OFF_STATE;
    switches[SWITCH_HASH_SET_ID].text = "Hash signing";
    switches[SWITCH_HASH_SET_ID].subText = "Enable hash signing";
    switches[SWITCH_HASH_SET_ID].token = SWITCH_HASH_SET_TOKEN;
    switches[SWITCH_HASH_SET_ID].tuneId = TUNE_TAP_CASUAL;

    switches[SWITCH_SEQUENCE_SET_ID].initState =
        (HAS_SETTING(S_SEQUENCE_NUMBER_ENABLED)) ? ON_STATE : OFF_STATE;
    switches[SWITCH_SEQUENCE_SET_ID].text = "Sequence number";
    switches[SWITCH_SEQUENCE_SET_ID].subText = "Display sequence in\ntransactions";
    switches[SWITCH_SEQUENCE_SET_ID].token = SWITCH_SEQUENCE_SET_TOKEN;
    switches[SWITCH_SEQUENCE_SET_ID].tuneId = TUNE_TAP_CASUAL;

    nbgl_useCaseHomeAndSettings(APPNAME,
                                &C_icon_stellar_64px,
                                NULL,
                                INIT_HOME_PAGE,
                                &setting_contents,
                                &info_list,
                                NULL,
                                app_quit);
}
#endif  // HAVE_NBGL
