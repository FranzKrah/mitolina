#ifndef HELPER_INDIVIDUALS_H
#define HELPER_INDIVIDUALS_H

#include "mitolina_types.h"

#include <vector>

bool find_path_from_root_to_dest(Individual* root, std::vector<Individual*>& path, const Individual* dest);

#endif
