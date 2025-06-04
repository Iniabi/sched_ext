#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/errno.h>

static int __init hmbird_patch_init(void)
{
    struct device_node *ver_np;
    const char *type;
    int ret;

    ver_np = of_find_node_by_path("/soc/oplus,hmbird/version_type");
    if (!ver_np) {
        pr_debug("hmbird_patch: version_type node not found\n");
        return 0;
    }

    ret = of_property_read_string(ver_np, "type", &type);
    if (ret) {
        pr_debug("hmbird_patch: type property not found\n");
        goto out_put_node;
    }

    if (strcmp(type, "HMBIRD_OGKI") != 0) {
        pr_debug("hmbird_patch: type is %s\n", type);
        goto out_put_node;
    }

    struct property *new_prop = kzalloc(sizeof(*new_prop), GFP_KERNEL);
    if (!new_prop) {
        pr_err("hmbird_patch: property alloc failed\n");
        ret = -ENOMEM;
        goto out_put_node;
    }

    new_prop->name = kstrdup("type", GFP_KERNEL);
    if (!new_prop->name) {
        pr_err("hmbird_patch: name alloc failed\n");
        ret = -ENOMEM;
        goto out_free_prop;
    }

    new_prop->value = kstrdup("HMBIRD_GKI", GFP_KERNEL);
    if (!new_prop->value) {
        pr_err("hmbird_patch: value alloc failed\n");
        ret = -ENOMEM;
        goto out_free_name;
    }
    new_prop->length = strlen("HMBIRD_GKI") + 1;

    ret = of_update_property(ver_np, new_prop);
    if (ret) {
        pr_err("hmbird_patch: update failed: %d\n", ret);
        goto out_free_value;
    }

    pr_info("hmbird_patch: updated to HMBIRD_GKI\n");
    goto out_put_node;

out_free_value:
    kfree(new_prop->value);
out_free_name:
    kfree(new_prop->name);
out_free_prop:
    kfree(new_prop);
out_put_node:
    of_node_put(ver_np);
    return ret;
}

early_initcall(hmbird_patch_init);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("reigadegr");
MODULE_DESCRIPTION("Convert HMBIRD_OGKI to HMBIRD_GKI");