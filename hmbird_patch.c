#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/string.h>

static int __init hmbird_patch_init(void)
{
    struct device_node *ver_np;
    const char *type;
    int ret;

    ver_np = of_find_node_by_path("/soc/oplus,hmbird/version_type");
    if (!ver_np) {
        pr_info("hmbird_patch: version_type node not found\n");
        return 0;
    }

    ret = of_property_read_string(ver_np, "type", &type);
    if (ret) {
        pr_info("hmbird_patch: type property not found\n");
        of_node_put(ver_np);
        return 0;
    }

    if (strcmp(type, "HMBIRD_OGKI")) {
        of_node_put(ver_np);
        return 0;
    }

    struct property *prop = of_find_property(ver_np, "type", NULL);
    if (!prop) {
        pr_info("hmbird_patch: type property structure not found\n");
        of_node_put(ver_np);
        return 0;
    }

    struct property *new_prop = kmalloc(sizeof(*new_prop), GFP_KERNEL);
    if (!new_prop) {
        pr_err("hmbird_patch: kmalloc for new_prop failed\n");
        of_node_put(ver_np);
        return -ENOMEM;
    }

    new_prop->name = prop->name;
    new_prop->length = strlen("HMBIRD_GKI") + 1;
    new_prop->value = kmalloc(new_prop->length, GFP_KERNEL);
    if (!new_prop->value) {
        pr_err("hmbird_patch: kmalloc for new_prop->value failed\n");
        kfree(new_prop);
        of_node_put(ver_np);
        return -ENOMEM;
    }

    strcpy(new_prop->value, "HMBIRD_GKI");

    if (of_remove_property(ver_np, prop) != 0) {
        pr_err("hmbird_patch: of_remove_property failed\n");
        kfree(new_prop->value);
        kfree(new_prop);
        of_node_put(ver_np);
        return -EIO;
    }

    if (of_add_property(ver_np, new_prop) != 0) {
        pr_err("hmbird_patch: of_add_property failed\n");
        kfree(new_prop->value);
        kfree(new_prop);
        of_node_put(ver_np);
        return -EIO;
    }

    pr_info("hmbird_patch: successfully converted from HMBIRD_OGKI to HMBIRD_GKI\n");
    of_node_put(ver_np);
    return 0;
}

early_initcall(hmbird_patch_init);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("reigadegr");
MODULE_DESCRIPTION("Forcefully convert HMBIRD_OGKI to HMBIRD_GKI.");