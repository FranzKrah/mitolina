#include "mitolina_types.h"

#include <vector>

class Individual {
private:
  int m_pid; 
  int m_generations_from_final = -1;
  bool m_is_female = true;
  
  std::vector<Individual*>* m_children = nullptr;
  Individual* m_mother = nullptr;
  
  Pedigree* m_pedigree = nullptr;
  int m_pedigree_id = 0;
  
  void meiosis_dist_tree_internal(Individual* dest, int* dist) const;
  
  bool m_dijkstra_visited = false;
  int m_dijkstra_distance = 0;

  std::vector<bool> m_haplotype;
  int m_haplotype_total_no_variants = -1; // for faster equality; if they do not have same number of variants, they cannot be equal
  bool m_haplotype_set = false;
  bool m_haplotype_mutated = false;
  void haplotype_mutate(std::vector<double>& mutation_rates);
  
  int m_haplotype_id = 0; // for faster equality
  
public:
  Individual(int pid, int generation, bool is_female);
  ~Individual();
  int get_pid() const;
  int get_generations_from_final() const;
  bool is_female() const;
  
  void add_child(Individual* child);
  //void set_mother(Individual* i);
  Individual* get_mother() const;
  std::vector<Individual*>* get_children() const;
  int get_children_count() const;
  bool pedigree_is_set() const;
  Pedigree* get_pedigree() const;
  int get_pedigree_id() const;
  
  void set_pedigree_id(int id, Pedigree* ped, int* pedigree_size);

  /*
  Called from pedigree destructor, only removes individual's knowledge about pedigree.
  The pedigree is responsible for removing this individuals from its list of individuals and
  relations.
  */
  void unset_pedigree();
  
  int meiosis_dist_tree(Individual* dest) const;
  
  std::vector<Individual*> calculate_path_to(Individual* dest) const;
  
  void dijkstra_reset();
  void dijkstra_tick_distance(int step);
  void dijkstra_set_distance_if_less(int dist);
  void dijkstra_mark_visited();
  int dijkstra_get_distance() const;
  bool dijkstra_was_visited() const;
  
  bool is_haplotype_set() const;
  void set_haplotype(std::vector<bool> h);
  void set_haplotype(std::vector<bool> h, int total_no_variants);
  std::vector<bool> get_haplotype() const;
  int get_haplotype_total_no_variants() const;
  void pass_haplotype_to_children(bool recursive, std::vector<double>& mutation_rates);
  
  void set_haplotype_id(const int id);
  int get_haplotype_id() const;
  
  int get_haplotype_L0(Individual* dest) const;
};

