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
#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "nbgl_use_case.h"

#include "display.h"
#include "globals.h"
#include "sw.h"
#include "stellar/printer.h"
#include "action/validate.h"

static void review_choice(bool confirm) {
    validate_pubkey(confirm);
    if (confirm) {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_VERIFIED, ui_menu_main);
    } else {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_REJECTED, ui_menu_main);
    }
}

int ui_display_address(void) {
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    explicit_bzero(G.ui.detail_value, DETAIL_VALUE_MAX_LENGTH);
    if (!print_account_id(G_context.raw_public_key,
                          G.ui.detail_value,
                          DETAIL_VALUE_MAX_LENGTH,
                          0,
                          0)) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    nbgl_useCaseAddressReview(G.ui.detail_value,
                              NULL,
                              &C_icon_stellar_64px,
                              "Verify Stellar address",
                              NULL,
                              review_choice);
    return 0;
}
#endif  // HAVE_NBGL
