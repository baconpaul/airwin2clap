//
// Created by Paul Walker on 6/19/22.
//

#include "claudio_configured.hxx"

#include <iostream>
#include <cmath>
#include <cstring>


static const clap_plugin *clap_create_plugin(const clap_plugin_factory *f, const clap_host *host,
                                             const char *plugin_id)
{
   auto fx = claudio_get_aeffInstance(host, plugin_id);
   std::cout << "FX = " << fx <<  " plugin_id = " << plugin_id << std::endl;

   return nullptr;
}

const CLAP_EXPORT struct clap_plugin_factory claudio_factory = {
        claudio_get_plugin_count,
        claudio_get_plugin_descriptor,
        clap_create_plugin,
};

static const void *get_factory(const char *factory_id) { return &claudio_factory; }

// clap_init and clap_deinit are required to be fast, but we have nothing we need to do here
bool clap_init(const char *p) { return true; }

void clap_deinit() {}

extern "C"
{
// clang-format off
const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
        CLAP_VERSION,
        clap_init,
        clap_deinit,
        get_factory
};
// clang-format on
}