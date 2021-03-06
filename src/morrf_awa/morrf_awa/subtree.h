#ifndef SUBTREE_H
#define SUBTREE_H

#include <vector>
#include <list>
#include <iostream>

#include "morrf_awa/KDTree2D.h"
#include "morrf_awa/objective_knn.h"

class MORRFNode;
class MORRF;

class RRTNode {
public:
    RRTNode( POS2D pos, int objective_num, int tree_idx );

    bool operator==( const RRTNode &other );

    int m_objective_num;
    std::vector<double> m_cost;
    double m_fitness;
    POS2D m_pos;
    RRTNode * mp_parent;
    MORRFNode* mp_host_node;

    int m_index;
    bool m_added;
    std::list<RRTNode*> m_child_nodes;
};

class Path {
public:
    Path( POS2D start, POS2D goal, int objectiveNum );
    bool is_dominated_by(Path* p_other_path);

    int m_objective_num;
    std::vector<double> m_cost;
    std::vector<double> m_weight;
    double m_fitness;
    POS2D m_start;
    POS2D m_goal;
    std::vector<POS2D> m_waypoints;
    int m_tree_idx;
    bool m_dominated;
    float m_sparsity_level;
};

class RRTree {
public:
    enum TREE_TYPE{ UNKNOWN, SUBPROBLEM, REFERENCE };
    RRTree( MORRF* parent, unsigned int objective_num, std::vector<float> weight, unsigned int index);
    virtual ~RRTree();

    RRTNode* init( POS2D start, POS2D goal );
    RRTNode* create_new_node( POS2D pos );
    bool remove_edge( RRTNode* p_node_p, RRTNode* p_node_c );
    bool has_edge( RRTNode* p_node_p, RRTNode* p_node_c );
    bool add_edge( RRTNode* p_node_p, RRTNode* p_node_c );

    RRTNode* find_nearest( POS2D pos );
    RRTNode* find_exact(POS2D pos);
    std::list<RRTNode*> find_near( POS2D pos );
    bool _contains( POS2D pos );

    std::list<RRTNode*> find_all_children( RRTNode* pNode );

    virtual void attach_new_node( RRTNode* p_node_new, std::list<RRTNode*> near_nodes ) = 0;
    virtual void rewire_near_nodes( RRTNode* p_node_new, std::list<RRTNode*> near_nodes ) = 0;
    virtual RRTNode * get_closet_to_goal( std::vector<double>& delta_cost, double& delta_fitness ) = 0;

    virtual void write_hist_data( std::ostream& out );

    bool is_structure_correct();
    bool are_all_nodes_tractable();
    bool are_all_nodes_fitness_positive();
    bool is_added_nodes_size_correct();

    void update_ball_radius();
    double get_ball_radius() { return _ball_radius; }

    RRTNode* find_ancestor( RRTNode* p_node );
    unsigned int get_current_iteration() { return m_nodes.size(); }

    Path* find_path(RRTNode* p_closet_node=NULL);
    void record();

    bool update_current_best(RRTNode* p_closet_node=NULL);
    Path* mp_current_best;
    std::vector<double> m_current_best_cost;
    double m_current_best_fitness;

    TREE_TYPE m_type;
    unsigned int m_index;
    unsigned int m_objective_num;

    POS2D m_start;
    POS2D m_goal;

    MORRF* mp_parent;
    RRTNode * mp_root;
    KDTree2D * mp_kd_tree;

    std::vector<double> m_weight;
    std::list<RRTNode*> m_nodes;
    std::list<RRTNode*> m_added_nodes;

    float m_sparsity_level;

    unsigned int m_first_path_iteration;
    std::vector< std::vector<double> > m_hist_cost;
    std::vector< double > m_hist_fitness;
    std::vector< double > m_hist_sparsity_level;

    double _ball_radius;
};

class ReferenceTree : public RRTree {
public:
    ReferenceTree( MORRF* parent, unsigned int objective_num, std::vector<float> weight, unsigned int index );

    virtual void attach_new_node( RRTNode* p_node_new, std::list<RRTNode*> near_nodes );
    virtual void rewire_near_nodes( RRTNode* p_node_new, std::list<RRTNode*> near_nodes );
    virtual RRTNode * get_closet_to_goal( std::vector<double>& delta_cost, double& delta_fitness );

    Path* find_path(RRTNode* p_closet_node=NULL);
    bool update_current_best(RRTNode* p_closet_node=NULL);
protected:
    void update_fitness_to_children( RRTNode* pNode, double delta_fitness );
};

class SubproblemTree : public RRTree {
public:
    SubproblemTree( MORRF* parent, unsigned int objective_num, std::vector<float> weight, unsigned int index );

    virtual void attach_new_node( RRTNode* p_node_new, std::list<RRTNode*> near_nodes );
    virtual void rewire_near_nodes( RRTNode* p_node_new, std::list<RRTNode*> near_nodes );
    virtual RRTNode * get_closet_to_goal( std::vector<double>& delta_cost, double& delta_fitness );

protected:
    void update_cost_to_children(RRTNode* p_node, std::vector<double>& delta_cost);
};

inline RRTNode* get_ancestor( RRTNode * p_node ) {
    if( NULL == p_node ) {
        return NULL;
    }
    if( NULL == p_node->mp_parent ) {
        return p_node;
    }
    else {
        return get_ancestor( p_node->mp_parent );
    }
}

inline void get_parent_node_list( RRTNode * p_node, std::list<RRTNode*>& path ) {
    if( p_node==NULL ) {
        return;
    }
    path.push_back( p_node );
    get_parent_node_list( p_node->mp_parent, path );
    return;
}

#endif // SUBTREE_H
