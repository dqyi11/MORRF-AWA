#include <cstdlib>
#include <iostream>
#include <fstream>
#include <limits>
#include "morrf_awa/morrf.h"
#include "morrf_awa/objective_knn.h"

#define OBSTACLE_THRESHOLD 200
#define DEFAULT_THETA      4.0;

using namespace std;

bool sparisity_compare_ascending(SubproblemTree* const & a, SubproblemTree* const & b) {
    return a->m_sparsity_level < b->m_sparsity_level;
}

bool sparisity_compare_descending(SubproblemTree* const & a, SubproblemTree* const & b) {
    return a->m_sparsity_level > b->m_sparsity_level;
}

MORRF::MORRF(unsigned int width, unsigned int height, unsigned int objective_num, unsigned int subproblem_num, unsigned int segmentLength, MORRF_TYPE type) {
    _sampling_width = width;
    _sampling_height = height;
    _objective_num = objective_num;
    _subproblem_num = subproblem_num;
    _type = type;
    _enable_init_weight_ws_transform = false;

    _range = ( _sampling_width > _sampling_height ) ? _sampling_width:_sampling_height;
    _obs_check_resolution = 1;
    _current_iteration = 0;
    _segment_length = segmentLength;

    _theta = DEFAULT_THETA;;
    _new_tree_creation_step = 20;
    _sparsity_k = 4; //_subproblem_num * 0.8;

    _pp_map_info = new int*[_sampling_width];
    for( unsigned int i=0; i<_sampling_width; i++ ) {
        _pp_map_info[i] = new int[_sampling_height];
        for( unsigned int j=0; j<_sampling_height; j++) {
            _pp_map_info[i][j] = 255;
        }
    }

    _p_kd_tree = new KDMORRFTree2D( std::ptr_fun(tac2) );

    _solution_available_iteration = -1;
    _solution_utopia = std::vector<double>(_objective_num, 0.0);
}

MORRF::~MORRF() {
    _deinit_weights();

}

void MORRF::add_funcs( std::vector<COST_FUNC_PTR> funcs, std::vector<int**> fitnessDistributions) {
    _funcs = funcs;
    _fitness_distributions = fitnessDistributions;
}

void MORRF::_init_weights( std::vector< std::vector<float> >& weights ) {
    _deinit_weights();
    bool auto_gen = false;
    if( weights.size() != _subproblem_num ) {
        auto_gen = true;
    }

    if(auto_gen) {
        _weights = create_weights( _subproblem_num );
    }
    else {
        _weights = weights;
    }

    if(_enable_init_weight_ws_transform) {
        std::cout << "saving weights before WS transformation" << std::endl;
        save_weights( _weights, "./before_weights.txt");
        _ws_weights = ws_transform( _weights );
        std::cout << "saving weights after WS transformation" << std::endl;
        save_weights( _ws_weights, "./after_weights.txt");
    }
}

void MORRF::_deinit_weights() {
    _weights.clear();
    _ws_weights.clear();
}

std::vector< std::vector< float > > MORRF::create_weights(unsigned int num) {
    std::vector< std::vector< float > > weights;

    srand(time(NULL));
    for( unsigned int i=0; i<num; i++ ) {
        vector<float> weight( _objective_num, 0.0 );
        std::vector<float> temp_array;
        temp_array.push_back(0.0);
        for( unsigned int j=0; j<_objective_num-1; j++ ) {
            temp_array.push_back( static_cast<float>(rand())/static_cast<float>(RAND_MAX) );
        }
        temp_array.push_back(1.0);
        sort(temp_array.begin(), temp_array.end());
        for( unsigned int j=0; j<_objective_num; j++ ) {
            weight[j] = temp_array[j+1] - temp_array[j];
        }
        weights.push_back( weight );
    }

    return weights;
}

void MORRF::init(POS2D start, POS2D goal, std::vector< std::vector<float> > weights) {

    m_start = start;
    m_goal = goal;

    _init_weights( weights );

    std::vector< std::vector<float> > weights4create;
    if(_enable_init_weight_ws_transform) {
        weights4create = _ws_weights;
    }
    else {
        weights4create = _weights;
    }

    _root_morrf_node = new MORRFNode(start);

    for( unsigned int k=0; k<_objective_num; k++ ) {
        create_reference_tree(k);

    }

    for( unsigned int m=0; m<_subproblem_num; m++ ) {
       create_subproblem_tree(weights4create[m], m+_objective_num);
    }

    _morrf_nodes.push_back(_root_morrf_node);
    _current_iteration = 0;
}

ReferenceTree* MORRF::create_reference_tree( unsigned int k ) {
    vector<float> weight(_objective_num, 0.0);
    weight[k] = 1.0;
    ReferenceTree * p_ref_tree = new ReferenceTree( this, _objective_num, weight, k );
    RRTNode * p_root_node = p_ref_tree->init( m_start, m_goal );
    p_root_node->mp_host_node = _root_morrf_node;
    _references.push_back( p_ref_tree );
    return p_ref_tree;
}

SubproblemTree* MORRF::create_subproblem_tree( std::vector<float>& weight, unsigned int index ) {
    SubproblemTree * p_sub_tree = new SubproblemTree( this, _objective_num, weight, index );
    RRTNode * p_root_node = p_sub_tree->init( m_start, m_goal );
    p_root_node->mp_host_node = _root_morrf_node;
    _subproblems.push_back( p_sub_tree );
    return p_sub_tree;
}

SubproblemTree* MORRF::create_subproblem_tree( std::vector<float>& weight, unsigned int index, std::vector<MORRFNode*>& pos_seq ) {
    SubproblemTree * p_sub_tree = new SubproblemTree( this, _objective_num, weight, index );
    RRTNode * p_root_node = p_sub_tree->init( m_start, m_goal );
    p_root_node->mp_host_node = _root_morrf_node;

    _subproblems.push_back( p_sub_tree );

    for(unsigned int i=0; i<pos_seq.size();i++) {
        MORRFNode* p_morrf_node = pos_seq[i];

        RRTNode* p_new_sub_node = p_sub_tree->create_new_node( p_morrf_node->m_pos );
        p_new_sub_node->mp_host_node = p_morrf_node;
        p_morrf_node->m_nodes.push_back( p_new_sub_node );
    }

    return p_sub_tree;
}

void MORRF::load_map(int **map) {
    for( unsigned int i=0; i<_sampling_width; i++ ) {
        for( unsigned int j=0; j<_sampling_height; j++ ) {
            _pp_map_info[i][j] = map[i][j];
        }
    }
}

POS2D MORRF::sampling() {
    double x = rand();
    double y = rand();
    x = x * ((double)(_sampling_width)/RAND_MAX);
    y = y * ((double)(_sampling_height)/RAND_MAX);

    POS2D m(x,y);
    return m;
}

POS2D MORRF::steer( POS2D pos_a, POS2D pos_b ) {
    POS2D new_pos( pos_a[0], pos_a[1] );
    double delta[2];
    delta[0] = pos_a[0] - pos_b[0];
    delta[1] = pos_a[1] - pos_b[1];
    double delta_len = std::sqrt( delta[0]*delta[0]+delta[1]*delta[1] );

    if ( delta_len > _segment_length ) {
        double scale = _segment_length / delta_len;
        delta[0] = delta[0] * scale;
        delta[1] = delta[1] * scale;

        new_pos.setX( pos_b[0]+delta[0] );
        new_pos.setY( pos_b[1]+delta[1] );
    }
    return new_pos;
}

bool MORRF::_is_in_obstacle( POS2D pos ) {
    int x = (int)pos[0];
    int y = (int)pos[1];
    if( _pp_map_info[x][y] < OBSTACLE_THRESHOLD )
        return true;
    return false;
}


bool MORRF::_is_obstacle_free( POS2D pos_a, POS2D pos_b ) {
    //if ( pos_a == pos_b ) {
    //    return true;
    //}
    int x_dist = (int)(pos_a[0] - pos_b[0]);
    int y_dist = (int)(pos_a[1] - pos_b[1]);

    if( x_dist == 0 && y_dist == 0 ) {
        return true;
    }

    float x1 = pos_a[0];
    float y1 = pos_a[1];
    float x2 = pos_b[0];
    float y2 = pos_b[1];

    const bool steep = ( fabs( y2 - y1 ) > fabs( x2 - x1 ) );
    if ( steep ) {
        std::swap( x1, y1 );
        std::swap( x2, y2 );
    }

    if ( x1 > x2 ) {
        std::swap( x1, x2 );
        std::swap( y1, y2 );
    }

    const float dx = x2 - x1;
    const float dy = fabs(y2 - y1);

    float error = dx / 2.0f;
    const int ystep = ( y1 < y2 ) ? 1 : -1;
    int y = (int)y1;

    const int maxX = (int)x2;

    for( int x=(int)x1; x<maxX; x++ ) {
        if( steep ) {
            if ( y >= 0 && y < _sampling_width && x >= 0 && x < _sampling_height ) {
                if ( _pp_map_info[y][x] < OBSTACLE_THRESHOLD ) {
                    return false;
                }
            }
        }
        else {
            if ( x >= 0 && x < _sampling_width && y >= 0 && y < _sampling_height ) {
                if ( _pp_map_info[x][y] < OBSTACLE_THRESHOLD ) {
                    return false;
                }
            }
        }

        error -= dy;
        if( error < 0 ) {
            y += ystep;
            error += dx;
        }
    }

    return true;
}

void MORRF::extend() {

    bool node_inserted = false;

    if( _solution_available_iteration >= 0 ) {

        // calculate sparsity level
        update_sparsity_level();

        if(_current_iteration > 0 && _current_iteration % _new_tree_creation_step == 0 ) {

            // find the max sparsity level
            std::vector<SubproblemTree*>::iterator max_it = std::max_element(_subproblems.begin(), _subproblems.end(), sparisity_compare_ascending);
            SubproblemTree* p_max_tree = (*max_it);
            if(p_max_tree) {

                std::vector<float> solution_mapping_vec(_objective_num, 0.0);
                for(unsigned int k=0;k<_objective_num;k++) {
                    solution_mapping_vec[k] = p_max_tree->m_current_best_cost[k] - _solution_utopia[k];
                }
                std::vector<float> new_weight = ws_transform(solution_mapping_vec);

                std::vector<MORRFNode*> pos_seq(_morrf_nodes.begin()+1,_morrf_nodes.begin()+_morrf_nodes.size());
                SubproblemTree* p_new_sub_tree = create_subproblem_tree( new_weight, _subproblems.size()+_objective_num, pos_seq );
                construct( pos_seq, p_new_sub_tree );
            }

            sort_subproblem_trees();
        }
    }

    while( false == node_inserted ) {

        // repair subproblem trees
        for( unsigned int m=0; m<_subproblem_num; m++ ) {
            SubproblemTree* p_sub_tree = _subproblems[m];
            if(p_sub_tree) {
                unsigned int mth_tree_size = p_sub_tree->m_added_nodes.size();
                if(mth_tree_size<_morrf_nodes.size()) {
                    std::cout << "REPAIR TREE " << p_sub_tree->m_index << std::endl;
                    std::vector<MORRFNode*> pos_seq(_morrf_nodes.begin()+mth_tree_size,_morrf_nodes.begin()+_morrf_nodes.size());
                    construct( pos_seq, p_sub_tree );
                }
            }
        }


        POS2D rnd_pos = sampling();
        KDMORRFNode2D nearest_node = find_nearest( rnd_pos );

        POS2D new_pos = steer( rnd_pos, nearest_node );

        if(true == _contains( new_pos )) {
            continue;
        }
        if( true == _is_in_obstacle( new_pos ) ) {
            continue;
        }

        if( true == _is_obstacle_free( nearest_node, new_pos )) {

            _sampled_positions.push_back(new_pos);

            node_inserted = true;
            MORRFNode* p_morrf_node = new MORRFNode(new_pos);


            for ( unsigned int m=0; m < _subproblems.size(); m++ ) {

            }

            _morrf_nodes.push_back( p_morrf_node );


            // create new nodes of reference trees
            // attach new node to reference trees
            // rewire near nodes of reference trees
            for ( unsigned int k=0; k<_objective_num; k++ ) {

                RRTNode * p_new_ref_node = _references[k]->create_new_node( new_pos );
                p_new_ref_node->mp_host_node = p_morrf_node;
                KDNode2D new_node( new_pos );
                new_node.mp_rrt_node = p_new_ref_node;
                _references[k]->mp_kd_tree->insert( new_node );
                p_morrf_node->m_nodes.push_back(p_new_ref_node);

                list<RRTNode*> near_ref_nodes = _references[k]->find_near( new_pos );

                _references[k]->attach_new_node( p_new_ref_node, near_ref_nodes );
                _references[k]->rewire_near_nodes( p_new_ref_node, near_ref_nodes );
            }

            // create new nodes of subproblem trees
            // attach new nodes to subproblem trees
            // rewire near nodes of subproblem trees
            for( unsigned int m=0; m<_subproblem_num; m++ ) {

                RRTNode * p_new_sub_node = _subproblems[m]->create_new_node( new_pos );
                p_new_sub_node->mp_host_node = p_morrf_node;

                KDNode2D new_node( new_pos );
                new_node.mp_rrt_node = p_new_sub_node;
                _subproblems[m]->mp_kd_tree->insert( new_node );

                list<RRTNode*> near_sub_nodes = _subproblems[m]->find_near( new_pos );

                _subproblems[m]->attach_new_node( p_new_sub_node, near_sub_nodes );
                _subproblems[m]->rewire_near_nodes( p_new_sub_node, near_sub_nodes );
            }
        }
    }

    update_ball_radius();
    // update current best
    update_current_best();

    if(_current_iteration % 10 == 0) {
        optimize();
    }
    record();    
    _current_iteration++;

    /*
    if(false == is_morrf_node_child_size_correct()) {
        std::cout << "morrf node size wrong" << std::endl;
    }
    for(unsigned int k=0;k<_objective_num;k++) {
        if(false==_references[k]->is_added_nodes_size_correct()) {
            std::cout << "ref tree " << k << " node size incorrect " << _references[k]->m_added_nodes.size() <<  std::endl;
        }
    }
    for(unsigned int m=0;m<_subproblem_num;m++) {
        if(false==_subproblems[m]->is_added_nodes_size_correct()) {
            std::cout << "sub tree " << m << " node size incorrect " << _subproblems[m]->m_added_nodes.size() <<  std::endl;
        }
    }*/
}

void MORRF::update_ball_radius() {
    int num_vertices = _morrf_nodes.size();
    int num_dimensions = 2;
    _ball_radius = get_theta() * get_range() * pow( log((double)(num_vertices + 1.0))/((double)(num_vertices + 1.0)), 1.0/((double)num_dimensions) );
}

void MORRF::update_sparsity_level() {

    unsigned int subproblem_num = _subproblems.size();
    float objs[ (_objective_num+subproblem_num) * _objective_num ];
    //memset(objs, 0, (_objective_num+_subproblem_num) * _objective_num);
    for( unsigned int k=0; k<_objective_num; k++ ) {
        for( unsigned int i=0; i<_objective_num; i++) {
            objs[k*_objective_num+i] =  _references[k]->m_current_best_cost[i];
        }
    }

    for( unsigned int m=0; m<subproblem_num; m++ ) {
        for( unsigned int i=0; i<_objective_num; i++) {
            objs[_objective_num*_objective_num+m*_objective_num+i] = _subproblems[m]->m_current_best_cost[i];
        }
    }

    flann::Matrix<float> obj_vec(objs, _objective_num+subproblem_num, _objective_num);
    ObjectiveKNN knn( _sparsity_k, obj_vec );
    std::vector<float> res = knn.get_sparse_diversity(obj_vec);

    for( unsigned int k=0; k<_objective_num; k++ ) {
        _references[k]->m_sparsity_level = res[k];
    }
    for( unsigned int m=0; m<subproblem_num; m++ ) {
        _subproblems[m]->m_sparsity_level = res[m+_objective_num];
    }
}

KDMORRFNode2D MORRF::find_nearest( POS2D pos ) {
    KDMORRFNode2D node( pos );

    std::pair<KDMORRFTree2D::const_iterator,double> found = _p_kd_tree->find_nearest( node );
    KDMORRFNode2D nearest_node = *found.first;
    return nearest_node;
}

KDMORRFNode2D MORRF::find_exact(POS2D pos) {
    KDMORRFNode2D node( pos );

    KDMORRFTree2D::const_iterator it = _p_kd_tree->find_exact( node );
    KDMORRFNode2D this_node = *it;
    return this_node;
}

std::list<KDMORRFNode2D> MORRF::find_near( POS2D pos ) {
    std::list<KDMORRFNode2D> near_list;
    KDMORRFNode2D node(pos);
    _p_kd_tree->find_within_range( node, get_ball_radius(), std::back_inserter(near_list) );

    return near_list;
}

bool MORRF::_contains( POS2D pos ) {
    if( _p_kd_tree ) {
        KDMORRFNode2D node( pos[0], pos[1] );
        KDMORRFTree2D::const_iterator it = _p_kd_tree->find( node );
        if( it != _p_kd_tree->end() ) {
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

void MORRF::update_sparsity_level( std::vector<Path*>& paths ) {

    unsigned int path_num = paths.size();
    float objs[ path_num * _objective_num ];

    for( unsigned int m=0; m<path_num; m++ ) {
        for( unsigned int i=0; i<_objective_num; i++) {
            objs[m*_objective_num+i] = paths[m]->m_cost[i];
        }
    }

    flann::Matrix<float> obj_vec(objs, path_num, _objective_num);
    ObjectiveKNN knn( _sparsity_k, obj_vec );
    std::vector<float> res = knn.get_sparse_diversity(obj_vec);

    for( unsigned int m=0; m<path_num; m++ ) {
        paths[m]->m_sparsity_level = res[m];
    }
}


bool MORRF::calc_cost(POS2D& pos_a, POS2D& pos_b, std::vector<double>& cost) {
    for( unsigned int k = 0; k < _objective_num; k++ ) {
        cost[k] = calc_kth_cost( pos_a, pos_b, k );
    }
    return true;
}

double MORRF::calc_kth_cost( POS2D& pos_a, POS2D& pos_b, unsigned int k ) {
    return _funcs[k]( pos_a, pos_b, _fitness_distributions[k], (void*)this );
}

double MORRF::calc_fitness( vector<double>& cost, vector<double>& weight, RRTNode* node ) {
    double fitness = 0.0;

    if( _type==MORRF::WEIGHTED_SUM ) {
        fitness = calc_fitness_by_weighted_sum( cost, weight );
    }
    else if( _type==MORRF::TCHEBYCHEFF ) {
        vector<double> utopia( _objective_num, 0.0 );
        get_utopia_reference_vector( node, utopia );
        fitness = calc_fitness_by_tchebycheff( cost, weight, utopia );
    }
    else {
        vector<double> utopia( _objective_num, 0.0 );
        get_utopia_reference_vector( node, utopia );
        fitness = calc_fitness_by_boundary_intersection( cost, weight, utopia );
    }
    /*
    if(fitness < 0.0) {
        std::cout << "Negative fitness " << fitness << std::endl;
    } */
    return fitness;
}

double MORRF::calc_fitness( vector<double>& cost, vector<double>& weight, POS2D& pos ) {
    double fitness = 0.0;

    if( _type==MORRF::WEIGHTED_SUM ) {
        fitness = calc_fitness_by_weighted_sum( cost, weight );
    }
    else if( _type==MORRF::TCHEBYCHEFF ) {
        vector<double> utopia(_objective_num, 0.0);
        get_utopia_reference_vector( pos, utopia );
        fitness = calc_fitness_by_tchebycheff( cost, weight, utopia );
    }
    else {
         vector<double> utopia(_objective_num, 0.0);
        get_utopia_reference_vector( pos, utopia );
        fitness = calc_fitness_by_boundary_intersection( cost, weight, utopia );
    }
    /*
    if(fitness < 0.0) {
        std::cout << "Negative fitness " << fitness << std::endl;
    } */
    return fitness;
}

double MORRF::calc_fitness( std::vector<double>& cost, std::vector<double>& weight, std::vector<double>& utopia ) {
    double fitness = 0.0;

    if( _type==MORRF::WEIGHTED_SUM ) {
        fitness = calc_fitness_by_weighted_sum( cost, weight );
    }
    else if( _type==MORRF::TCHEBYCHEFF ) {
        fitness = calc_fitness_by_tchebycheff( cost, weight, utopia );
    }
    else {
        fitness = calc_fitness_by_boundary_intersection( cost, weight, utopia );
    }
    /*
    if(fitness < 0.0) {
        std::cout << "Negative fitness " << fitness << std::endl;
    } */
    return fitness;
}

float MORRF::calc_fitness_by_weighted_sum( vector<double>& cost, vector<double>& weight ) {
    double fitness = 0.0;
    for( unsigned int k=0; k<_objective_num; k++ ) {
        fitness += cost[k] * weight[k];
    }
    return fitness;
}

float MORRF::calc_fitness_by_tchebycheff( vector<double>& cost, vector<double>& weight, vector<double>& utopia_reference ) {
    std::vector<float> weighted_distance(_objective_num, 0.0);
    for( unsigned int k=0; k<_objective_num; k++ ) {
       weighted_distance[k] = weight[k] * fabs( cost[k] - utopia_reference[k] );
    }
    sort(weighted_distance.begin(), weighted_distance.end());
    return weighted_distance.back();
}

float MORRF::calc_fitness_by_boundary_intersection( vector<double>& cost, vector<double>& weight, vector<double>& utopia_reference ) {
    double d1 = 0.0, d2 = 0.0;
    for( unsigned int k=0; k<_objective_num; k++ ) {
       double weighted_dist = weight[k] * (cost[k] - utopia_reference[k]);
       d1 += weighted_dist;
    }
    d1 = fabs(d1);
    vector<double> vectorD2(_objective_num, 0.0);
    for( unsigned int k=0; k<_objective_num; k++ ) {
        vectorD2[k] = cost[k] - (utopia_reference[k] + d1* weight[k]);
        d2 += vectorD2[k]*vectorD2[k];
    }
    d2 = sqrt(d2);
    return d1 + _theta * d2;
}


bool MORRF::get_utopia_reference_vector(POS2D&  pos, vector<double>& utopia ) {

    KDMORRFNode2D ref_node = find_nearest(pos);
    if( ref_node.mp_morrf_node->m_nodes.size()<_objective_num ) {
        return false;
    }

    for( unsigned int k=0; k<_objective_num; k++ ) {
        RRTNode* p_RRT_node = ref_node.mp_morrf_node->m_nodes[k];
        utopia[k] = p_RRT_node->m_fitness;
    }
    return true;
}

bool MORRF::get_utopia_reference_vector( RRTNode* p_node, vector<double>& utopia ) {
    if( p_node == NULL ) {
         return false;
    }
    if( p_node && p_node->mp_host_node ) {
        for( unsigned int k=0; k<_objective_num; k++ ) {
            utopia[k] = p_node->mp_host_node->m_nodes[k]->m_fitness;
        }
    }
    return true;
}

ReferenceTree* MORRF::get_reference_tree(unsigned int k) {
    if( k<0 || k>=_references.size() ) {
        return NULL;
    }
    return _references[k];
}

SubproblemTree* MORRF::get_subproblem_tree( unsigned int m ) {
    if( m<0 || m>=_subproblems.size() ) {
        return NULL;
    }
    return _subproblems[m];
}

void MORRF::optimize() {
    if(_p_kd_tree) {
        _p_kd_tree->optimize();
    }
}

void MORRF::dump_map_info( std::string filename ) {
    ofstream map_info_file;
    map_info_file.open(filename.c_str());
    if( _pp_map_info ) {
        for( unsigned int i=0; i<_sampling_width; i++ ) {
            for( unsigned int j=0; j<_sampling_height; j++ ) {
                map_info_file << _pp_map_info[i][j] << " ";
            }
            map_info_file << std::endl;
        }
    }
    map_info_file.close();
}

void MORRF::dump_weights( std::string filename ) {
  save_weights(_weights, filename);
}

void MORRF::save_weights( std::vector< std::vector<float> >& weights, std::string filename ) {
  ofstream weight_file;
  weight_file.open(filename.c_str());

  for( unsigned int i=0; i<weights.size(); i++ ) {
      std::vector<float> w = weights[i];
      for( unsigned int j=0; j<w.size(); j++ ) {
          weight_file << w[j] << " ";
      }
      weight_file << std::endl;
  }

  weight_file.close();
}

bool MORRF::are_reference_structures_correct() {
    for( vector<ReferenceTree*>::iterator it=_references.begin(); it!=_references.end(); it++ ) {
        ReferenceTree* p_ref_tree = (*it);
        if( p_ref_tree ) {
            if( false == p_ref_tree->is_structure_correct() ) {
                return false;
            }
        }
    }
    return true;
}

bool MORRF::are_subproblem_structures_correct() {
    for( vector<SubproblemTree*>::iterator it=_subproblems.begin(); it!=_subproblems.end(); it++ ) {
        SubproblemTree* p_sub_tree = (*it);
        if( p_sub_tree ) {
            if( false == p_sub_tree->is_structure_correct() ) {
                return false;
            }
        }
    }
    return true;
}

bool MORRF::are_all_reference_nodes_tractable() {
    for( vector<ReferenceTree*>::iterator it=_references.begin(); it!=_references.end(); it++ ) {
        ReferenceTree* p_ref_tree = (*it);
        if( p_ref_tree ) {
            if( false == p_ref_tree->are_all_nodes_tractable() ) {
                return false;
            }
        }
    }
    return true;
}

bool MORRF::are_all_subproblem_nodes_tractable() {
    for( vector<SubproblemTree*>::iterator it=_subproblems.begin(); it!=_subproblems.end(); it++ ) {
        SubproblemTree* p_sub_tree = (*it);
        if( p_sub_tree ) {
            if( false == p_sub_tree->are_all_nodes_tractable() ) {
                return false;
            }
        }
    }
    return true;
}

bool MORRF::are_all_reference_nodes_fitness_positive() {
    for( std::vector<ReferenceTree*>::iterator it=_references.begin(); it!=_references.end(); it++ ) {
        ReferenceTree* p_ref_tree = (*it);
        if( p_ref_tree ) {
            if( false == p_ref_tree->are_all_nodes_fitness_positive() ) {
                return false;
            }
        }
    }
    return true;
}

bool MORRF::are_all_subproblem_nodes_fitness_positive() {
    for( vector<SubproblemTree*>::iterator it=_subproblems.begin(); it!=_subproblems.end(); it++ ) {
        SubproblemTree* p_sub_tree = (*it);
        if( p_sub_tree ) {
            if( false == p_sub_tree->are_all_nodes_fitness_positive() ) {
                return false;
            }
        }
    }
    return true;
}

bool MORRF::is_node_number_identical() {
    unsigned int ref_num = _references[0]->m_nodes.size();

    for(vector<ReferenceTree*>::iterator it=_references.begin();it!=_references.end();it++) {
        ReferenceTree* p_ref_tree = (*it);
        if(p_ref_tree) {
            unsigned int num = p_ref_tree->m_nodes.size();
            if(num != ref_num) {
                return false;
            }
        }
    }
    for(vector<SubproblemTree*>::iterator it=_subproblems.begin();it!=_subproblems.end();it++) {
        SubproblemTree* p_sub_tree = (*it);
        if(p_sub_tree) {
            unsigned int num = p_sub_tree->m_nodes.size();
            if(num != ref_num) {
                return false;
            }
        }
    }
    return true;
}

vector<Path*> MORRF::get_paths() {
    vector<Path*> paths;

    for(vector<ReferenceTree*>::iterator it=_references.begin();it!=_references.end();it++) {
        ReferenceTree* p_ref_tree = (*it);
        if(p_ref_tree) {
            Path* pRefPath = p_ref_tree->mp_current_best;
            if(pRefPath) {
              paths.push_back(pRefPath);
            }
        }
    }
    /*
    for(unsigned int m=0;m<_subproblem_num;m++) {
        if(_subproblems[m]->mp_current_best) {
            paths.push_back(_subproblems[m]->mp_current_best);
        }
    }
    update_dominance(paths);
    */
    vector<Path*> paths_from_subproblem_trees;
    for(unsigned int m=0;m<_subproblems.size();m++) {
        if(_subproblems[m]->mp_current_best) {
            paths_from_subproblem_trees.push_back(_subproblems[m]->mp_current_best);
        }
    }
    update_dominance(paths_from_subproblem_trees);
    unsigned int tree_idx = 0;
    while(paths.size()<_objective_num+_subproblem_num && tree_idx < _subproblems.size()) {
        Path* p_curr_path = _subproblems[tree_idx]->mp_current_best;
        if(p_curr_path) {
            //if(p_curr_path->m_dominated==false) {
                paths.push_back(p_curr_path);
            //}
        }
        tree_idx++;
    }
    update_sparsity_level(paths);
    return paths;
}

bool MORRF::update_path_cost( Path *p ) {
    if(p) {
        for(unsigned int k=0;k<_objective_num;k++) {
            p->m_cost[k] = 0.0;
        }
        for(unsigned int i=0;i<p->m_waypoints.size()-1;i++) {
            POS2D pos_a = p->m_waypoints[i];
            POS2D pos_b = p->m_waypoints[i+1];
            vector<double> delta_cost(_objective_num, 0.0);
            calc_cost(pos_a, pos_b, delta_cost);

            for(unsigned int k=0;k<_objective_num;k++) {
                p->m_cost[k] += delta_cost[k];
            }
        }
        return true;
    }
    return false;
}

bool MORRF::update_current_best() {

    KDMORRFNode2D nearest_node = find_nearest( m_goal );
    if( _is_obstacle_free(m_goal, nearest_node) == false ) {
        return false;
    }
    if( _solution_available_iteration < 0 ) {
        _solution_available_iteration = _current_iteration;
    }

    for( unsigned int k=0; k<_objective_num; k++ ) {
        ReferenceTree* p_ref_tree = _references[k];
        if(p_ref_tree) {
            unsigned int index = p_ref_tree->m_index;
            RRTNode* p_closest_node = nearest_node.mp_morrf_node->m_nodes[index];
            p_ref_tree->update_current_best(p_closest_node);
            _solution_utopia[k] = p_ref_tree->m_current_best_cost[k];
            p_ref_tree->m_current_best_fitness = _solution_utopia[k];
        }
    }

    for( unsigned int m=0; m<_subproblem_num; m++ ) {
        SubproblemTree* p_sub_tree = _subproblems[m];
        if(p_sub_tree) {

            RRTNode* p_closest_node = p_sub_tree->find_nearest( m_goal );
            p_sub_tree->update_current_best(p_closest_node);
            p_sub_tree->m_current_best_fitness = calc_fitness(p_sub_tree->m_current_best_cost,
                                                              p_sub_tree->m_weight,
                                                              _solution_utopia);
        }
    }
    return true;
}

void MORRF::construct( vector<MORRFNode*>& pos_seq, vector<SubproblemTree*>& new_subproblems ) {

    std::cout << "CONSTRUCT " << pos_seq.size() << std::endl;

    for(unsigned int i=0; i<pos_seq.size();i++) {
        MORRFNode* p_morrf_node = pos_seq[i];
        POS2D current_pos = p_morrf_node->m_pos;
        for(unsigned int j=0; j<new_subproblems.size();j++) {
            SubproblemTree* p_sub_tree = new_subproblems[j];

            if(p_sub_tree) {

                RRTNode* p_new_sub_node = p_sub_tree->create_new_node( current_pos );
                p_new_sub_node->mp_host_node = p_morrf_node;

                std::list<RRTNode*> near_sub_nodes =  p_sub_tree->find_near( current_pos );

                p_sub_tree->attach_new_node( p_new_sub_node,  near_sub_nodes );
                p_sub_tree->rewire_near_nodes( p_new_sub_node, near_sub_nodes );

                p_sub_tree->update_current_best();
                p_sub_tree->record();
            }
        }
    }

    /*
    for(unsigned int m=0; m<new_subproblems.size();m++) {
        SubproblemTree* p_sub_tree = new_subproblems[m];
        if(p_sub_tree) {
            p_sub_tree->update_current_best();
        }
    }*/
}

void MORRF::construct( std::vector<MORRFNode*>& pos_seq,  SubproblemTree* p_new_sub_tree ) {

    //std::cout << "CONSTRUCT " << pos_seq.size() << std::endl;
    if(p_new_sub_tree) {
        for(unsigned int i=0; i<pos_seq.size();i++) {
            MORRFNode* p_morrf_node = pos_seq[i];
            POS2D current_pos = p_morrf_node->m_pos;

            RRTNode* p_new_sub_node = p_new_sub_tree->create_new_node( current_pos );
            p_new_sub_node->mp_host_node = p_morrf_node;

            std::list<RRTNode*> near_sub_nodes =  p_new_sub_tree->find_near( current_pos );

            p_new_sub_tree->attach_new_node( p_new_sub_node,  near_sub_nodes );
            p_new_sub_tree->rewire_near_nodes( p_new_sub_node, near_sub_nodes );

            p_new_sub_tree->update_current_best();
            p_new_sub_tree->record();
        }
    }
}

void MORRF::dump_subproblem_sparsity( std::string filename ) {
    ofstream sparsity_file;
    sparsity_file.open(filename.c_str());

    for( unsigned int i=0; i<_subproblems.size(); i++ ) {
        sparsity_file << _subproblems[i]->m_index << " ";
    }
    sparsity_file << std::endl;
    sparsity_file << std::endl;

    for( unsigned int j=0; j<_subproblems.size(); j++ ) {
        for( unsigned int i=0; i<_subproblem_num; i++ ) {
            sparsity_file << _subproblems[i]->m_current_best_cost[j] << " ";
        }
        sparsity_file << std::endl;
    }

    sparsity_file << std::endl;

    for( unsigned int i=0; i<_subproblems.size(); i++ ) {
        sparsity_file << _subproblems[i]->m_sparsity_level << " ";
    }

    sparsity_file.close();
}

std::vector< SubproblemTree* > MORRF::add_subproblem_trees( unsigned int num ) {

    std::vector< SubproblemTree* > trees = std::vector< SubproblemTree* >(num, NULL);

    // sampling weights
    std::vector< std::vector< float > > weights = create_weights(num);

    // create trees
    unsigned int index = _objective_num + _subproblems.size();
    for(unsigned int i=0;i<num;i++) {
        trees[i] = new SubproblemTree(this, _objective_num, weights[i], index);
        RRTNode * p_root_node = trees[i]->init( m_start, m_goal );
        p_root_node->m_added = true;
        _morrf_nodes[0]->m_nodes.push_back(p_root_node);
        /*
        std::cout << "weight (" << i << ")" ;
        for(unsigned int k=0;k<_objective_num;k++) {
            std::cout << trees[i]->m_weight[k] << " ";
        }
        std::cout << std::endl;*/
        index ++;
    }

    // merge trees
    for(unsigned int i=0;i<num;i++) {
        _subproblems.push_back(trees[i]);
        _weights.push_back(weights[i]);
    }

    std::cout << "MORRF node size " << _morrf_nodes.size() << std::endl;
    // expand trees
    std::vector<MORRFNode*>::const_iterator first = _morrf_nodes.begin()+1;
    std::vector<MORRFNode*>::const_iterator last = _morrf_nodes.begin()+_morrf_nodes.size();
    std::vector<MORRFNode*> seq(first, last);
    construct( seq,  trees );

    return trees;
}

std::vector< std::vector< float > > MORRF::ws_transform( std::vector< std::vector< float > >& weights ) {
  std::vector< std::vector< float > > new_weights;
  for(std::vector< std::vector< float> >::iterator it = weights.begin();
      it != weights.end(); it++) {
    std::vector< float > w = (*it);
    std::vector< float > new_w = ws_transform( w );
    new_weights.push_back( new_w );
  }
  return new_weights;
}

std::vector< float > MORRF::ws_transform( std::vector< float >& weight ) {
  std::vector< float > inv_weight(weight.size(), 0.0);
  std::vector< float > new_weight(weight.size(), 0.0);
  double weight_sum = 0.0;
  for(unsigned int i=0; i<weight.size(); i++) {
    inv_weight[i] = 1.0/weight[i];
    weight_sum += inv_weight[i];
  }
  for(unsigned int i=0; i<inv_weight.size(); i++) {
    new_weight[i] = inv_weight[i]/weight_sum;
  }
  return new_weight;
}

void MORRF::record() {
    for(vector<ReferenceTree*>::iterator it=_references.begin();it!=_references.end();it++) {
        ReferenceTree* p_ref_tree = (*it);
        if(p_ref_tree) {
            p_ref_tree->record();
        }
    }
    for(vector<SubproblemTree*>::iterator it=_subproblems.begin();it!=_subproblems.end();it++) {
        SubproblemTree* p_sub_tree = (*it);
        if(p_sub_tree) {
            p_sub_tree->record();
        }
    }

}

void MORRF::write_hist_cost(std::string filename) {
    ofstream hist_cost_file;
    hist_cost_file.open(filename.c_str());

    for(unsigned int k=0;k<_objective_num;k++) {
        ReferenceTree* p_ref_tree = _references[k];
        if(p_ref_tree) {
            p_ref_tree->write_hist_data(hist_cost_file);
        }
    }
    std::vector<double> subprob_fitness(_subproblems.size(), 0.0);
    for(unsigned int m=0;m<_subproblems.size();m++) {
        SubproblemTree* p_sub_tree = _subproblems[m];
        if(p_sub_tree) {
            p_sub_tree->write_hist_data(hist_cost_file);
        }
    }
    hist_cost_file.close();
}

void MORRF::update_dominance( std::vector<Path*>& paths ) {

    for(std::vector<Path*>::iterator it=paths.begin();
        it!=paths.end();it++) {
        Path* p_path = (*it);
        p_path->m_dominated = false;
        for(std::vector<Path*>::iterator itc=paths.begin();
            itc!=paths.end();itc++) {
            Path* p_other_path = (*itc);
            if(p_path != p_other_path) {
                if(p_path->is_dominated_by(p_other_path)) {
                    p_path->m_dominated = true;
                    break;
                }
            }
        }
    }
}

void MORRF::sort_subproblem_trees() {
    std::sort(_subproblems.begin(), _subproblems.end(), sparisity_compare_descending);
}
