cmake_minimum_required (VERSION 2.6)

compile_i18n_po_to_mo(messages.po messages.mo)
install_i18n_catalogue(messages.po share/xm7/C)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES messages.mo)