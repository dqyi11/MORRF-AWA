#ifndef MORRF_H
#define MORRF_H

#include "morrf_awa/subtree.h"

typedef double (*COST_FUNC_PTR)(POS2D, POS2D, int**, void*);

class MORRFNode {
public:
    MORRFNode( POS2D pos ) {
        m_pos = pos;
    }

    POS2D m_pos;
    std::vector< RRTNode*> m_nodes;
};

class MORRF {
    friend class SubproblemTree;
    friend class ReferenceTree;
public:
    enum MORRF_TYPE{ WEIGHTED_SUM = 0, TCHEBYCHEFF, BOUNDARY_INTERSACTION };
    MORRF( unsigned int width, unsigned int height, unsigned int objective_num, unsigned int subproblem_num, unsigned int segment_length, MORRF_TYPE type=WEIGHTED_SUM );
    ~MORRF();

    void add_funcs( std::vector<COST_FUNC_PTR> funcs, std::vector<int**> fitnessDistributions );

    void init(POS2D start, POS2D goal, std::vector< std::vector<float> > weights = std::vector< std::vector<float> >(0));

    void load_map( int **pp_map );
    POS2D sampling();
    POS2D steer( POS2D pos_a, POS2D pos_b );
    void extend();

    bool _is_obstacle_free( POS2D pos_a, POS2D pos_b );
    bool _is_in_obstacle( POS2D pos );


    bool calc_cost( POS2D& pos_a, POS2D& pos_b, std::vector<double>& cost );
    double calc_kth_cost( POS2D& pos_a, POS2D& pos_b, unsigned int k );
    double calc_fitness( std::vector<double>& cost, std::vector<double>& weight, RRTNode* node );
    double calc_fitness( std::vector<double>& cost, std::vector<double>& weight, POS2D& pos );
    double calc_fitness( std::vector<double>& cost, std::vector<double>& weight, std::vector<double>& utopia );

    bool get_utopia_reference_vector( POS2D& pos, std::vector<double>& utopia );
    bool get_utopia_reference_vector( RRTNode* p_node, std::vector<double>& utopia );

    int get_sampling_width() { return _sampling_width; }
    int get_sampling_height() { return _sampling_height; }

    void set_obstacle_info(int ** pp_obstacle) { _pp_map_info = pp_obstacle; }

    int get_current_iteration() { return _current_iteration; }

    ReferenceTree* get_reference_tree( unsigned int k );
    SubproblemTree* get_subproblem_tree( unsigned int m );
    unsigned int get_subproblem_tree_num() { return _subproblems.size(); }

    KDMORRFNode2D find_nearest( POS2D pos );
    KDMORRFNode2D find_exact(POS2D pos);
    std::list<KDMORRFNode2D> find_near( POS2D pos );
    bool _contains( POS2D pos );

    std::vector<Path*> get_paths();

    bool update_current_best();
    void update_dominance( std::vector<Path*>& paths );

    int** get_map_info() { return _pp_map_info; }

    void dump_map_info( std::string filename );
    void dump_weights( std::string filename );
    void save_weights( std::vector< std::vector<float> >& weights, std::string filename );

    void dump_subproblem_sparsity( std::string filename );

    float calc_fitness_by_weighted_sum( std::vector<double>& cost, std::vector<double>& weight );
    float calc_fitness_by_tchebycheff( std::vector<double>& cost, std::vector<double>& weight, std::vector<double>& utopia_reference );
    float calc_fitness_by_boundary_intersection( std::vector<double>& cost, std::vector<double>& weight, std::vector<double>& utopia_reference );

    bool are_reference_structures_correct();
    bool are_subproblem_structures_correct();
    bool are_all_reference_nodes_tractable();
    bool are_all_subproblem_nodes_tractable();
    bool are_all_reference_nodes_fitness_positive();
    bool are_all_subproblem_nodes_fitness_positive();
    bool is_node_number_identical();

    bool update_path_cost( Path *p );

    void construct( std::vector<MORRFNode*>& pos_seq,  std::vector<SubproblemTree*>& new_subproblems );
    void construct( std::vector<MORRFNode*>& pos_seq,  SubproblemTree* p_new_sub_tree );

    void optimize();

    void set_sparsity_k(unsigned int k) { _sparsity_k = k; }
    unsigned int get_sparsity_k() { return _sparsity_k; }

    void record();
    void write_hist_cost(std::string filename);

    void set_init_weight_ws_transform(bool enable) { _enable_init_weight_ws_transform = enable; }
    bool get_init_weight_ws_transform() { return _enable_init_weight_ws_transform; }

    std::vector< SubproblemTree* > add_subproblem_trees( unsigned int num );
    std::vector< std::vector< float > > create_weights(unsigned int num);
    std::vector< std::vector< float > > ws_transform( std::vector< std::vector< float > >& weights );
    std::vector< float > ws_transform( std::vector< float >& weight );

    void sort_subproblem_trees();

    void update_ball_radius();
    double get_ball_radius() { return _ball_radius; }
    double get_range() { return _range; }
    double get_theta() { return _theta; }
    void set_theta( double theta ) { _theta = theta; }
protected:
    void _init_weights( std::vector< std::vector<float> >& weights );
    void _deinit_weights();

    void update_sparsity_level();
    void update_sparsity_level( std::vector<Path*>& paths );
    ReferenceTree* create_reference_tree( unsigned int k );
    SubproblemTree* create_subproblem_tree( std::vector<float>& weight, unsigned int index );
    SubproblemTree* create_subproblem_tree( std::vector<float>& weight, unsigned int index, std::vector<MORRFNode*>& pos_seq );

private:
    int ** _pp_map_info;

    MORRF_TYPE _type;
    unsigned int _sampling_width;
    unsigned int _sampling_height;

    unsigned int _objective_num;
    unsigned int _subproblem_num;


    std::vector<COST_FUNC_PTR> _funcs;
    std::vector<int**> _fitness_distributions;

    std::vector< std::vector<float> > _weights;
    std::vector< std::vector<float> > _ws_weights;

    std::vector<SubproblemTree*> _subproblems;
    std::vector<ReferenceTree*> _references;

    std::vector<POS2D> _sampled_positions;

    POS2D m_start;
    POS2D m_goal;

    double _range;
    double _segment_length;
    int _obs_check_resolution;
    bool _enable_init_weight_ws_transform;

    std::vector<MORRFNode*> _morrf_nodes;
    double _ball_radius;

    MORRFNode* _root_morrf_node;
    double _theta;
    int _current_iteration;
    int _new_tree_creation_step;

    unsigned int _sparsity_k;
    int _solution_available_iteration;
    std::vector<double> _solution_utopia;

    KDMORRFTree2D * _p_kd_tree;
};

#endif // MORRF_H
