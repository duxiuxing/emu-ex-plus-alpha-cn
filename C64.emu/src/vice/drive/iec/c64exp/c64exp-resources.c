/*
 * c64exp-resources.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>

#include "c64exp-resources.h"
#include "drive.h"
#include "drivemem.h"
#include "lib.h"
#include "profdos.h"
#include "resources.h"
#include "stardos-exp.h"
#include "supercard.h"
#include "uiapi.h"
#include "util.h"
#include "userport.h"
#include "init.h"


static char *profdos_1571_name = NULL;
static char *supercard_name = NULL;
static char *stardos_name = NULL;

static int set_drive_parallel_cable(int val, void *param)
{
    diskunit_context_t *unit = diskunit_context[vice_ptr_to_uint(param)];
    int userport_device = -1;

    switch (val) {
        case DRIVE_PC_NONE:
        case DRIVE_PC_STANDARD:
        case DRIVE_PC_DD3:
        case DRIVE_PC_FORMEL64:
        case DRIVE_PC_21SEC_BACKUP:
            break;
        default:
            return -1;
    }

    unit->parallel_cable = val;
    /* don't reset CMDHD drives */
    if (unit->type != DRIVE_TYPE_CMDHD) {
        drivemem_init(unit);
    }

    /* some magic to automatically insert or remove the parallel cable into/from the user port */
    resources_get_int("UserportDevice", &userport_device);

    if ((val == DRIVE_PC_NONE) && (userport_device == USERPORT_DEVICE_DRIVE_PAR_CABLE)) {
        int hasparcable = 0;
        int dnr;
        /* check if any drive has a parallel cable enabled */
        for (dnr = 0; dnr < NUM_DISK_UNITS; dnr++) {
            int cable;
            resources_get_int_sprintf("Drive%iParallelCable", &cable, dnr + 8);
            if (cable != DRIVE_PC_NONE) {
                hasparcable = 1;
            }
        }
        /* if no drive uses parallel cable, disable it in the userport settings */
        if (hasparcable == 0) {
            resources_set_int("UserportDevice", USERPORT_DEVICE_NONE);
        }
    } else if (val != DRIVE_PC_NONE) {
        if (userport_device == USERPORT_DEVICE_NONE) {
            resources_set_int("UserportDevice", USERPORT_DEVICE_DRIVE_PAR_CABLE);
        } else if (userport_device != USERPORT_DEVICE_DRIVE_PAR_CABLE) {
            if (init_main_is_done()) {
                ui_message("Warning: the user port is already being used for another device.\n"
                            "To be able to use the parallel cable, you must also set up the user port accordingly.");
            }
        }
    }

    return 0;
}

static int set_drive_profdos(int val, void *param)
{
    diskunit_context_t *unit = diskunit_context[vice_ptr_to_uint(param)];

    unit->profdos = val ? 1 : 0;
    drivemem_init(unit);

    return 0;
}

static int set_profdos_1571_name(const char *val, void *param)
{
    if (util_string_set(&profdos_1571_name, val)) {
        return 0;
    }

    return profdos_load_1571(profdos_1571_name);
}

static int set_drive_supercard(int val, void *param)
{
    diskunit_context_t *unit = diskunit_context[vice_ptr_to_uint(param)];

    unit->supercard = val ? 1 : 0;
    drivemem_init(unit);

    return 0;
}

static int set_supercard_name(const char *val, void *param)
{
    if (util_string_set(&supercard_name, val)) {
        return 0;
    }

    return supercard_load(supercard_name);
}

static int set_drive_stardos(int val, void *param)
{
    diskunit_context_t *unit = diskunit_context[vice_ptr_to_uint(param)];

    unit->stardos = val ? 1 : 0;

    return 0;
}

static int set_stardos_name(const char *val, void *param)
{
    if (util_string_set(&stardos_name, val)) {
        return 0;
    }

    return stardos_exp_load(stardos_name);
}

static resource_int_t res_drive[] = {
    { NULL, DRIVE_PC_NONE, RES_EVENT_SAME, NULL,
      NULL, set_drive_parallel_cable, NULL },
    { NULL, 0, RES_EVENT_SAME, NULL,
      NULL, set_drive_profdos, NULL },
    { NULL, 0, RES_EVENT_SAME, NULL,
      NULL, set_drive_supercard, NULL },
    { NULL, 0, RES_EVENT_SAME, NULL,
      NULL, set_drive_stardos, NULL },
    RESOURCE_INT_LIST_END
};

static const resource_string_t resources_string[] =
{
    { "DriveProfDOS1571Name", "", RES_EVENT_NO, NULL,
      &profdos_1571_name, set_profdos_1571_name, NULL },
    { "DriveSuperCardName", "", RES_EVENT_NO, NULL,
      &supercard_name, set_supercard_name, NULL },
    { "DriveStarDosName", "", RES_EVENT_NO, NULL,
      &stardos_name, set_stardos_name, NULL },
    RESOURCE_STRING_LIST_END
};

int c64exp_resources_init(void)
{
    int dnr;

    for (dnr = 0; dnr < NUM_DISK_UNITS; dnr++) {
        diskunit_context_t *unit = diskunit_context[dnr];

        res_drive[0].name = lib_msprintf("Drive%iParallelCable", dnr + 8);
        res_drive[0].value_ptr = &(unit->parallel_cable);
        res_drive[0].param = uint_to_void_ptr(dnr);
        res_drive[1].name = lib_msprintf("Drive%iProfDOS", dnr + 8);
        res_drive[1].value_ptr = &(unit->profdos);
        res_drive[1].param = uint_to_void_ptr(dnr);
        res_drive[2].name = lib_msprintf("Drive%iSuperCard", dnr + 8);
        res_drive[2].value_ptr = &(unit->supercard);
        res_drive[2].param = uint_to_void_ptr(dnr);
        res_drive[3].name = lib_msprintf("Drive%iStarDos", dnr + 8);
        res_drive[3].value_ptr = &(unit->stardos);
        res_drive[3].param = uint_to_void_ptr(dnr);

        if (resources_register_int(res_drive) < 0) {
            return -1;
        }

        lib_free(res_drive[0].name);
        lib_free(res_drive[1].name);
        lib_free(res_drive[2].name);
        lib_free(res_drive[3].name);
    }

    return resources_register_string(resources_string);
}

void c64exp_resources_shutdown(void)
{
    lib_free(profdos_1571_name);
    lib_free(supercard_name);
    lib_free(stardos_name);
}
