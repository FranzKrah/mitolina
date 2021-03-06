#include "mitolina_types.h"

#include <RcppArmadillo.h> // FIXME: Avoid Rcpp here? Only in api_* files?

/*
==========================================
Pedigree
==========================================
*/

Pedigree::Pedigree(int id) {
  //Rcpp::Rcout << "Pedigree with id = " << id << " created" << std::endl;
  
  m_pedigree_id = id;
  
  m_all_individuals = new std::vector<Individual*>();
  m_relations = new std::vector< std::pair<Individual*, Individual*>* >();
}

Pedigree::~Pedigree() {
  //Rcpp::Rcout << "   CLEANUP PEDIGREE!\n";
  
  for (auto it = m_all_individuals->begin(); it != m_all_individuals->end(); ++it) {
    /*
    Only unsets individual's knowledge about pedigree.
    This pedigree is responsible for removing this individuals from its list of individuals and
    relations.
    */
    (*it)->unset_pedigree();
  }  
  delete m_all_individuals;
  
  for (auto it = m_relations->begin(); it != m_relations->end(); ++it) {
    delete *it;
  }
  
  delete m_relations;
}

int Pedigree::get_id() const {
  return m_pedigree_id;
}

void Pedigree::add_member(Individual* i) {
  m_all_individuals->push_back(i);
}

void Pedigree::add_relation(Individual* lhs, Individual* rhs) {
  //std::pair<Individual*, Individual*>* pair = new std::pair(lhs, rhs);
  std::pair<Individual*, Individual*>* pair = new std::pair<Individual*, Individual*>(lhs, rhs);
  m_relations->push_back(pair);
}

std::vector<Individual*>* Pedigree::get_all_individuals() const {
  return m_all_individuals;
}

std::vector< std::pair<Individual*, Individual*>* >* Pedigree::get_relations() const {
  return m_relations;
}



Individual* Pedigree::get_root() {
  if (m_root != nullptr) {
    return m_root;
  }
  
  /* FIXME: Exploits tree */
  bool root_set = false;
  
  for (auto &individual : (*m_all_individuals)) {
    if (individual->get_mother() == nullptr) {
      if (root_set) {
        Rcpp::stop("Only expected one root in male pedigree!");
      } else {
        m_root = individual;
        root_set = true;
      }
      
      break;
    }
  }

  if (!root_set || m_root == nullptr) {
    Rcpp::stop("Expected a root in male pedigree!");
  }

  return m_root;
}


void Pedigree::populate_haplotypes(int loci, std::vector<double>& mutation_rates) {
  /* FIXME: Exploits tree */
  Individual* root = this->get_root();
  
  std::vector<bool> h(loci); // initialises to 0, 0, ..., 0 / false, false, ..., false
  
  root->set_haplotype(h, 0);
  root->pass_haplotype_to_children(true, mutation_rates);
}

void Pedigree::populate_haplotypes_custom_founders(std::vector<double>& mutation_rates, Rcpp::Function get_founder_hap) {
  /* FIXME: Exploits tree */
  Individual* root = this->get_root();
  
  std::vector<bool> h = Rcpp::as< std::vector<bool> >( get_founder_hap() );  
  
  // Test that a haplotype of proper length generated  
  if (h.size() != mutation_rates.size()) {
    Rcpp::stop("get_founder_haplotype generated haplotype with number of loci different from the number of mutation rates specified");
  }
  
  //Rf_PrintValue(Rcpp::wrap(h));
  
  int no_variants = 0;
  
  for (size_t i = 0; i < h.size(); ++i) {
    if (h[i] == true) {
      no_variants += 1;
    }
  }
  
  root->set_haplotype(h, no_variants);
  root->pass_haplotype_to_children(true, mutation_rates);
}



