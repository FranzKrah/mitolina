#include "mitolina_types.h"

#include <stdexcept>

#include <RcppArmadillo.h> // FIXME: Avoid Rcpp here? Only in api_* files?

/*
==========================================
Individual
==========================================
*/
Individual::Individual(int pid, int generations_from_final, bool is_female) {
  m_pid = pid;
  m_generations_from_final = generations_from_final;
  m_is_female = is_female;
  
  m_children = new std::vector<Individual*>();
}

Individual::~Individual() {
  delete m_children;
}

int Individual::get_pid() const {
  return m_pid;
}

int Individual::get_generations_from_final() const {
  return m_generations_from_final;
}

bool Individual::is_female() const {
  return m_is_female;
}

void Individual::add_child(Individual* child) {
  child->m_mother = this; // child->set_mother(this);
  m_children->push_back(child);
}

/*
void Individual::set_mother(Individual* i) {
  // FIXME: Check sex of i?
  m_mother = i;
}
*/

Individual* Individual::get_mother() const {
  return m_mother;
}

std::vector<Individual*>* Individual::get_children() const {
  return m_children;
}

int Individual::get_children_count() const {
  return m_children->size();
}

bool Individual::pedigree_is_set() const {
  return (m_pedigree_id != 0);
}

int Individual::get_pedigree_id() const {
  return m_pedigree_id;
}

Pedigree* Individual::get_pedigree() const {
  return m_pedigree;
}

void Individual::unset_pedigree() {
  if (!this->pedigree_is_set()) {
    return;
  }
  
  m_pedigree = nullptr;
  m_pedigree_id = 0;
}

// Recursively assigns children
void Individual::set_pedigree_id(int id, Pedigree* ped, int* pedigree_size) {
  if (this->pedigree_is_set()) {
    return;
  }
  
  m_pedigree = ped;
  m_pedigree_id = id;
  *pedigree_size += 1;
  ped->add_member(this);
  
  if (m_mother != nullptr) {  
    m_mother->set_pedigree_id(id, ped, pedigree_size);
  }
  
  for (auto &child : (*m_children)) {
    ped->add_relation(this, child);
    child->set_pedigree_id(id, ped, pedigree_size);
  }
}

void Individual::dijkstra_reset() {
  m_dijkstra_visited = false;
  m_dijkstra_distance = 0;
}

void Individual::dijkstra_tick_distance(int step) {
  m_dijkstra_distance += step;
}
  
void Individual::dijkstra_set_distance_if_less(int dist) {
  if (m_dijkstra_distance < dist) {
    m_dijkstra_distance = dist;
  }
}

void Individual::dijkstra_mark_visited() {
  m_dijkstra_visited = true;
}

int Individual::dijkstra_get_distance() const {
  return m_dijkstra_distance; 
}

bool Individual::dijkstra_was_visited() const {
  return m_dijkstra_visited; 
}

// ASSUMES TREE!
//FIXME: Heavily relies on it being a tree, hence there is only one path connecting every pair of nodes
void Individual::meiosis_dist_tree_internal(Individual* dest, int* dist) const {
  if (this->get_pid() == dest->get_pid()) {
    //FIXME: Heavily relies on it being a tree, hence there is only one path connecting every pair of nodes
    *dist = dest->dijkstra_get_distance();    
    return;
  }
  
  if (dest->dijkstra_was_visited()) {
    return;
  }
  
  dest->dijkstra_mark_visited();
  dest->dijkstra_tick_distance(1);
  int m = dest->dijkstra_get_distance();
  
  Individual* mother = dest->get_mother();
  if (mother != nullptr) {  
    //tree: ok
    mother->dijkstra_tick_distance(m);
    
    // general? FIXME Correct?
    //mother->dijkstra_set_distance_if_less(m);
    
    this->meiosis_dist_tree_internal(mother, dist); 
  }
  
  std::vector<Individual*>* children = dest->get_children();
  for (auto child : *children) {
    //tree: ok
    child->dijkstra_tick_distance(m);

    // general? FIXME Correct?
    //child->dijkstra_set_distance_if_less(m);
    
    this->meiosis_dist_tree_internal(child, dist);
  }
}

// ASSUMES TREE!
int Individual::meiosis_dist_tree(Individual* dest) const {
  if (!(this->pedigree_is_set())) {
    throw std::invalid_argument("!(this->pedigree_is_set())");
  }
  
  if (dest == nullptr) {
    throw std::invalid_argument("dest is NULL");
  }
  
  if (!(dest->pedigree_is_set())) {
    throw std::invalid_argument("!(dest->pedigree_is_set())");
  }
  
  if (this->get_pedigree_id() != dest->get_pedigree_id()) {
    return -1;
  }
  
  // At this point, the individuals this and dest belong to same pedigree
    
  std::vector<Individual*>* inds = this->get_pedigree()->get_all_individuals();
  for (auto child : *inds) {
    child->dijkstra_reset();
  }

  int dist = 0;
  this->meiosis_dist_tree_internal(dest, &dist);
  return dist;
}



/*
mother haplotype
FIXME mutation_model?
*/
void Individual::haplotype_mutate(std::vector<double>& mutation_rates) {
  if (!m_haplotype_set) {
    throw std::invalid_argument("mother haplotype not set yet, so cannot mutate");
  }
  if (m_haplotype.size() != mutation_rates.size()) {
    throw std::invalid_argument("Number of loci specified in haplotype must equal number of mutation rates specified");
  }
  if (m_haplotype_mutated) {
    throw std::invalid_argument("mother haplotype already set and mutated");
  }  
  if (m_haplotype_total_no_variants == -1) {
    throw std::invalid_argument("m_haplotype_total_no_variants not set");
  }
  
  for (int loc = 0; loc < m_haplotype.size(); ++loc) {
    if (R::runif(0.0, 1.0) < mutation_rates[loc]) {
      if (m_haplotype[loc] == false) {
        m_haplotype[loc] = true; // flip bit / mutate locus
        m_haplotype_total_no_variants += 1;
      } else { // m_haplotype[loc] == true
        m_haplotype[loc] = false; // flip bit / mutate locus
        m_haplotype_total_no_variants -= 1;
      }
    }
  }
}

bool Individual::is_haplotype_set() const {
  return m_haplotype_set; 
}

void Individual::set_haplotype(std::vector<bool> h) {
  m_haplotype = h;
  m_haplotype_set = true;
}

void Individual::set_haplotype(std::vector<bool> h, int total_no_variants) {
  this->set_haplotype(h);
  m_haplotype_total_no_variants = total_no_variants;
}

std::vector<bool> Individual::get_haplotype() const {
  return m_haplotype;
}

int Individual::get_haplotype_total_no_variants() const {
  return m_haplotype_total_no_variants;
}

void Individual::pass_haplotype_to_children(bool recursive, std::vector<double>& mutation_rates) {
  for (auto &child : (*m_children)) {
    child->set_haplotype(m_haplotype, this->get_haplotype_total_no_variants());
    child->haplotype_mutate(mutation_rates);
    
    if (recursive) {
      child->pass_haplotype_to_children(recursive, mutation_rates);
    }
  }
}

int Individual::get_haplotype_L0(Individual* dest) const {
  std::vector<bool> h_this = this->get_haplotype();
  std::vector<bool> h_dest = dest->get_haplotype();
  
  if (h_this.size() != h_dest.size()) {
    Rcpp::Rcout << "this pid = " << this->get_pid() << " has haplotype with " << h_this.size() << " loci" << std::endl;
    Rcpp::Rcout << "dest pid = " << dest->get_pid() << " has haplotype with " << h_dest.size() << " loci" << std::endl;
    throw std::invalid_argument("h_this.size() != h_dest.size()");
  }
  
  int d = 0;
  for (size_t i = 0; i < h_this.size(); ++i) {
    if (h_this[i] == h_dest[i]) {
      continue;
    }
    
    d += 1;
  }
  
  return d;
}

std::vector<Individual*> Individual::calculate_path_to(Individual* dest) const {
  if (!(this->pedigree_is_set())) {
    throw std::invalid_argument("!(this->pedigree_is_set())");
  }
  
  if (dest == nullptr) {
    throw std::invalid_argument("dest is NULL");
  }
  
  if (!(dest->pedigree_is_set())) {
    throw std::invalid_argument("!(dest->pedigree_is_set())");
  }
  
  if (this->get_pedigree_id() != dest->get_pedigree_id()) {
    std::vector<Individual*> empty_vec;
    return empty_vec;
  }
  
  // At this point, the individuals this and dest belong to same pedigree
  
  Individual* root = this->get_pedigree()->get_root();
  
  std::vector<Individual*> path_this, path_dest;

  if (!find_path_from_root_to_dest(root, path_this, this)) {
    Rcpp::Rcout << "this pid = " << this->get_pid() << std::endl;  
    throw std::invalid_argument("Could not find path between root and this");
  }
  
  if (!find_path_from_root_to_dest(root, path_dest, dest)) {
    Rcpp::Rcout << "dest pid = " << dest->get_pid() << std::endl;
    throw std::invalid_argument("Could not find path between root and dest");
  }
  
  int LCA_index = 0;
  for (LCA_index = 0; LCA_index < path_this.size() && LCA_index < path_dest.size(); LCA_index++) {
    if (path_this[LCA_index]->get_pid() != path_dest[LCA_index]->get_pid()) {
      break;
    }
  }
  
  if (LCA_index == 0) {
    throw std::invalid_argument("LCA_index cannot be 0");
  }
  
  std::vector<Individual*> path_result;
  path_result.push_back(path_this[LCA_index - 1]); // LCA = path_this[LCA_index - 1] == path_dest[LCA_index - 1]
  path_result.insert(path_result.end(), path_this.begin() + LCA_index, path_this.end());
  path_result.insert(path_result.end(), path_dest.begin() + LCA_index, path_dest.end());
  
  return path_result;
}



void Individual::set_haplotype_id(const int id) {
  m_haplotype_id = id;
}

int Individual::get_haplotype_id() const {
  if (m_haplotype_id == 0) {
    throw std::invalid_argument("haplotype id not yet inferred");
  }
  
  return m_haplotype_id;
}

