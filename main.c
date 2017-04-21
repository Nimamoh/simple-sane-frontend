#include <sane/sane.h>
#include <stdio.h>
#include <stdlib.h>

static int count; /** Number of devices*/
int count_devices(const SANE_Device ***ptr);

int options_to_auto(SANE_Handle handle);

SANE_Device *get_device(const SANE_Device ***devices, int index);

int main() {

    //INIT
    SANE_Status last_status = SANE_STATUS_GOOD;
    SANE_Int version = 1;
    last_status = sane_init(&version, NULL);
    if (last_status != SANE_STATUS_GOOD) goto return_1;

    //DEVICES
    const SANE_Device **device_list = malloc(sizeof(SANE_Device) * 256);
    last_status = sane_get_devices(&device_list, SANE_FALSE);

    if (last_status != SANE_STATUS_GOOD) goto failed_device_list;

    //OPEN THE SECOND DEVICE
    SANE_Device *device = get_device(device_list, 1); //XXX device selected manually.
    SANE_Handle handle = NULL;
    last_status = sane_open(device->name, &handle);
    if (last_status != SANE_STATUS_GOOD) goto failed_handle;

    //PARAMETERS
    SANE_Parameters parameters;
    last_status = sane_get_parameters(handle, &parameters);
    if (last_status != SANE_STATUS_GOOD) goto failed_parameters;

    last_status = SANE_STATUS_GOOD;
    while (last_status == SANE_STATUS_GOOD) {

        options_to_auto(handle);

        //START
        last_status = sane_start(handle);
        if (last_status != SANE_STATUS_GOOD) goto failed_start;

        //READ
        const SANE_Int max_len = parameters.bytes_per_line;
        SANE_Byte *data = malloc(sizeof(SANE_Byte) * max_len);
        SANE_Int len = 0;
        last_status = sane_read(handle, data, max_len, &len);

        for (int i = 0; i < len; ++i) {
            SANE_Byte byte = data[i];
            if (byte == EOF)
                printf("Hit EOF!");
        }
    }

    goto return_0;

    failed_parameters:
    failed_start:
    failed_handle:
    sane_close(handle);

    failed_device_list:
    free(device_list);

    return_1:
    {
        SANE_String_Const status_str = sane_strstatus(last_status);
        fprintf(stderr, "%s\n", status_str);
    }
    sane_exit();
    return 1;

    return_0:
    sane_close(handle);
    sane_exit();
    return 0;
}

int count_devices(const SANE_Device ***ptr) {
    int i = 0;
    while (ptr[i] != NULL) ++i;
    return i;
}

/** @return 0 if success, 1 if failure*/
int options_to_auto(SANE_Handle handle) {

    //Retrieve option number
    int i = 0;
    const SANE_Option_Descriptor *option_desc = sane_get_option_descriptor(handle, i);
    if (option_desc == NULL || option_desc->type != SANE_TYPE_INT)
        return 1;
    SANE_Int nb_params = 0;
    SANE_Int infos = 0;
    sane_control_option(handle, i, SANE_ACTION_GET_VALUE, &nb_params, &infos);
    i++;

    //Set all options to auto
    for (i; i < nb_params; ++i) {
        const SANE_Option_Descriptor *option_desc = sane_get_option_descriptor(handle, i);
        if (option_desc == NULL) continue;

        printf("Setting %s: %s to auto.\n", option_desc->name, option_desc->desc, option_desc->constraint_type);
        switch(option_desc->constraint_type){
            case SANE_CONSTRAINT_WORD_LIST:
            {
                printf("\tConstraint word list\n");
                int j = 0;
                SANE_Word word = option_desc->constraint.word_list[j];
                while(word != NULL) {
                    word = option_desc->constraint.word_list[j];
                    printf("\t\tWord: %i\n", word);
                    j++;
                }
            }
                break;
            case SANE_CONSTRAINT_STRING_LIST:
            {
                printf("\tConstraint string list\n");
                int j = 0;
                SANE_String_Const str = option_desc->constraint.string_list[j];
                while(str != NULL) {
                    str = option_desc->constraint.string_list[j];
                    printf("\t\t%s\n", str);
                    j++;
                }
                option_desc->constraint.string_list;
            }

                break;
            case SANE_CONSTRAINT_RANGE:
                printf("\tConstraint range\n");
                printf("\t\tFrom %i to %i\n", option_desc->constraint.range->min, option_desc->constraint.range->max);
                break;
            case SANE_CONSTRAINT_NONE:
            default:
                printf("\tNo constraint ;)\n");
                break;
        }
        SANE_Int *info = NULL;
        sane_control_option(handle, i, SANE_ACTION_SET_AUTO, NULL, info);
    }
}

SANE_Device *get_device(const SANE_Device ***devices, int index) {
    if (count == 0) {
        count = count_devices(devices);
    }

    if (index >= count)
        return NULL;
    else
        return devices[index];
}