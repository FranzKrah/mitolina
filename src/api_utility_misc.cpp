#include <RcppArmadillo.h>

// [[Rcpp::depends(RcppProgress)]]
#include <progress.hpp>

#include <string>

#include "mitolina_types.hpp"

//[[Rcpp::export]]
void mitolina_test() {
  Rcpp::Rcout << "mikl was here 1111" << std::endl;
}

//[[Rcpp::export]]
int pop_size(Rcpp::XPtr<Population> population) {
  return population->get_population_size();
}


//' @export
// [[Rcpp::export]]
Rcpp::IntegerMatrix meioses_generation_distribution(Rcpp::XPtr<Individual> individual, int generation_upper_bound_in_result = -1) {  
  Individual* i = individual;
  
  Pedigree* ped = i->get_pedigree();
  std::vector<Individual*>* family = ped->get_all_individuals();
  std::map<int, std::map<int, int> > tab;
  
  for (auto dest : *family) {    
    int generation = dest->get_generations_from_final();
    
    if (generation_upper_bound_in_result != -1 && generation > generation_upper_bound_in_result) {
      continue;
    }
    
    int dist = i->meiosis_dist_tree(dest);

    (tab[generation])[dist] += 1;    
  }
  
  int row = 0;
  for (auto const& x1 : tab) {
    for (auto const& x2 : x1.second) {
      ++row;
    }
  }
  Rcpp::IntegerMatrix res(row, 3);
  colnames(res) = Rcpp::CharacterVector::create("generation", "meioses", "count");
  row = 0;
  for (auto const& x1 : tab) {
    for (auto const& x2 : x1.second) {
      res(row, 0) = x1.first;
      res(row, 1) = x2.first;
      res(row, 2) = x2.second;
      ++row;    
    }
  }
  
  return res;
}




//' @export
// [[Rcpp::export]]
int population_size_generation(Rcpp::XPtr<Population> population, bool is_female = true, int generation_upper_bound_in_result = -1) {  
  std::unordered_map<int, Individual*>* pop = population->get_population();
  
  int size = 0;
  
  for (auto dest : *pop) {    
    int generation = dest.second->get_generations_from_final();
    
    if (generation_upper_bound_in_result != -1 && generation > generation_upper_bound_in_result) {
      continue;
    }
    
    if (dest.second->is_female() != is_female) {
      continue;
    }
    
    ++size;
  }
  
  return size;
}


//' @export
// [[Rcpp::export]]
int pedigree_size_generation(Rcpp::XPtr<Pedigree> pedigree, int generation_upper_bound_in_result = -1) {  
  std::vector<Individual*>* family = pedigree->get_all_individuals();
  
  int size = 0;
  
  for (auto dest : *family) {    
    int generation = dest->get_generations_from_final();
    
    if (generation_upper_bound_in_result != -1 && generation > generation_upper_bound_in_result) {
      continue;
    }
    
    ++size;
  }
  
  return size;
}





