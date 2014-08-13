cmake_minimum_required (VERSION 2.6)

compile_i18n_po_to_mo(ja.po ja.mo)
install_i18n_catalogue(ja.po share/xm7/ja)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES messages.mo)
