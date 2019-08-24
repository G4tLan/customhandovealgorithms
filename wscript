# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):

    lte_module_dependencies = ['core', 'network', 'spectrum', 'stats', 'buildings', 'virtual-net-device','point-to-point','applications','internet','csma']
    if (bld.env['ENABLE_EMU']):
        lte_module_dependencies.append('fd-net-device')
    module = bld.create_ns3_module('customhandovealgorithms', lte_module_dependencies)
    module.source = [
        'model/algorithmAdam.cc',
        'model/songMoonAlgorithm.cc',
        'helper/UE.cc',
        'helper/Enbs.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'customhandovealgorithms'
    headers.source = [
        'model/algorithmAdam.h',
        'model/songMoonAlgorithm.h',
        'helper/UE.h',
        'helper/Enbs.h',
        ]

    # bld.ns3_python_bindings()

