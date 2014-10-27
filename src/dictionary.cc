/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
/*
COPYING CONDITIONS NOTICE:

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation, and provided that the
  following conditions are met:

      * Redistributions of source code must retain this COPYING
        CONDITIONS NOTICE, the COPYRIGHT NOTICE (below), the
        DISCLAIMER (below), the UNIVERSITY PATENT NOTICE (below), the
        PATENT MARKING NOTICE (below), and the PATENT RIGHTS
        GRANT (below).

      * Redistributions in binary form must reproduce this COPYING
        CONDITIONS NOTICE, the COPYRIGHT NOTICE (below), the
        DISCLAIMER (below), the UNIVERSITY PATENT NOTICE (below), the
        PATENT MARKING NOTICE (below), and the PATENT RIGHTS
        GRANT (below) in the documentation and/or other materials
        provided with the distribution.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

COPYRIGHT NOTICE:

  TokuFT, Tokutek Fractal Tree Indexing Library.
  Copyright (C) 2007-2013 Tokutek, Inc.

DISCLAIMER:

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

UNIVERSITY PATENT NOTICE:

  The technology is licensed by the Massachusetts Institute of
  Technology, Rutgers State University of New Jersey, and the Research
  Foundation of State University of New York at Stony Brook under
  United States of America Serial No. 11/760379 and to the patents
  and/or patent applications resulting from it.

PATENT MARKING NOTICE:

  This software is covered by US Patent No. 8,185,551.
  This software is covered by US Patent No. 8,489,638.

PATENT RIGHTS GRANT:

  "THIS IMPLEMENTATION" means the copyrightable works distributed by
  Tokutek as part of the Fractal Tree project.

  "PATENT CLAIMS" means the claims of patents that are owned or
  licensable by Tokutek, both currently or in the future; and that in
  the absence of this license would be infringed by THIS
  IMPLEMENTATION or by using or running THIS IMPLEMENTATION.

  "PATENT CHALLENGE" shall mean a challenge to the validity,
  patentability, enforceability and/or non-infringement of any of the
  PATENT CLAIMS or otherwise opposing any of the PATENT CLAIMS.

  Tokutek hereby grants to you, for the term and geographical scope of
  the PATENT CLAIMS, a non-exclusive, no-charge, royalty-free,
  irrevocable (except as stated in this section) patent license to
  make, have made, use, offer to sell, sell, import, transfer, and
  otherwise run, modify, and propagate the contents of THIS
  IMPLEMENTATION, where such license applies only to the PATENT
  CLAIMS.  This grant does not include claims that would be infringed
  only as a consequence of further modifications of THIS
  IMPLEMENTATION.  If you or your agent or licensee institute or order
  or agree to the institution of patent litigation against any entity
  (including a cross-claim or counterclaim in a lawsuit) alleging that
  THIS IMPLEMENTATION constitutes direct or contributory patent
  infringement, or inducement of patent infringement, then any rights
  granted to you under this License shall terminate as of the date
  such litigation is filed.  If you or your agent or exclusive
  licensee institute or order or agree to the institution of a PATENT
  CHALLENGE, then Tokutek may terminate any rights granted to you
  under this License.
*/

#ident "Copyright (c) 2007-2013 Tokutek Inc.  All rights reserved."
#ident "The technology is licensed by the Massachusetts Institute of Technology, Rutgers State University of New Jersey, and the Research Foundation of State University of New York at Stony Brook under United States of America Serial No. 11/760379 and to the patents and/or patent applications resulting from it."
#ident "$Id$"

#include <ctype.h>

#include <db.h>
#include "dictionary.h"

void dictionary::create(const char* dname) {
    m_dname = toku_strdup(dname);
}

void dictionary::destroy(){
    toku_free(m_dname);
}

const char* dictionary::get_dname() {
    return m_dname;
}



void dictionary_manager::create() {
    ZERO_STRUCT(m_mutex);
    toku_mutex_init(&m_mutex, nullptr);
    m_dictionary_map.create();
}

void dictionary_manager::destroy() {
    m_dictionary_map.destroy();
    toku_mutex_destroy(&m_mutex);
}

int dictionary_manager::find_by_dname(dictionary *const &dbi, const char* const &dname) {
    return strcmp(dbi->get_dname(), dname);
}

dictionary* dictionary_manager::find(const char* dname) {
    dictionary *dbi;
    int r = m_dictionary_map.find_zero<const char*, find_by_dname>(dname, &dbi, nullptr);
    return r == 0 ? dbi : nullptr;
}

void dictionary_manager::add_db(dictionary* dbi) {
    int r = m_dictionary_map.insert<const char*, find_by_dname>(dbi, dbi->get_dname(), nullptr);
    invariant_zero(r);
}

void dictionary_manager::remove_dictionary(dictionary* dbi) {
    toku_mutex_lock(&m_mutex);
    uint32_t idx;
    dictionary *found_dbi;
    int r = m_dictionary_map.find_zero<const char*, find_by_dname>(
        dbi->get_dname(),
        &found_dbi,
        &idx
        );
    invariant_zero(r);
    invariant(found_dbi == dbi);
    r = m_dictionary_map.delete_at(idx);
    invariant_zero(r);
    toku_mutex_unlock(&m_mutex);
}

dictionary* dictionary_manager::get_dictionary(const char * dname) {
    toku_mutex_lock(&m_mutex);
    dictionary *dbi = find(dname);
    if (dbi == nullptr) {
        XCALLOC(dbi);
        dbi->create(dname);
    }
    toku_mutex_unlock(&m_mutex);
    return dbi;
}
