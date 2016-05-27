#ifndef OBJECTIVE_KNN_H_
#define OBJECTIVE_KNN_H_

#include <flann/flann.hpp>

class ObjectiveVector : public flann::Matrix<float>{
public:

    ObjectiveVector( double*  p_val, int dimension) {
        _dimension = dimension;
        _value = new float[_dimension];
        for(unsigned int i=0; i<_dimension;i++) {
            _value[i] = p_val[i];
        }
        flann::Matrix<int>(d, 1, _dimension);
    }

    ObjectiveVector( const ObjectiveNode & x ) {
        for(unsigned int i=0; i<_dimension;i++) {
            _value[i] = x._value[i];
        }
        flann::Matrix<float>(x.ptr(), 1, _dimension);
    }

    ~ObjectiveVector(){
        if(_value) {
            delete _value;
            _value = NULL;
        }
    }

    double distance_to(ObjectiveVector const& x) const  {
        double dist = 0;
        for (unsigned int i = 0; i < _dimension; i++) {
            dist += (_value[i]-x._value[i])*(_value[i]-x._value[i]);
        }
        return std::sqrt(dist);
    }

    float operator[]( size_t const N ) {
        return _value[N];
    }

    bool operator==(const ObjectiveVector &other) const {
        for (unsigned int i = 0; i < _dimension; i++) {
            if(_value[i]!=other._value[i]) {
                return false;
            }
        }
        return true;
    }

    int get_dimension() {
        return _dimension;
    }

protected:
    float* _value;
    int    _dimension;
};

class ObjectiveNode : public ObjectiveVector {
public:
    KDNode2D( value_type x, value_type y ) : POS2D( x, y ) { m_node_list.clear(); }
    KDNode2D( POS2D & pos ) : POS2D( pos ) { m_node_list.clear(); }

    std::vector<RRTNode*> m_node_list;
};


inline std::ostream& operator<< ( std::ostream& out, ObjectiveVector const& T ) {
    out << '(' ;
    for(unsigned int i=0; i<T.get_dimension();i++) {
        if(i==0) {
            out << T[i];
        }
        else{
            out << ',' << T[i];
        }
    }
    out << ')';
    return out;
}

class ObjectiveKNN {
public:
    ObjectiveKNN() {
        _p_index = NULL;
    }

    virtual ~ObjectiveKNN() {
        if(_p_index) {
            delete _p_index;
            _p_index = NULL;
        }
    }

    void insert( ObjectiveNode node ) {
        if( _p_index == NULL ) {
            _p_index = new flann::Index<flann::L2<float> >( node, flann::KDTreeIndexParams(4) );
        }
        else {
            _p_index->addPoints( node );
        }
    }

    size_t size() const {
        return _p_index->size();
    }

    void find_within_range( ObjectiveVector pos, float range, std::list<ObjectiveNode>& node_list ) {
        if( _p_index ) {
            std::vector<std::vector<int> > indices;
            std::vector<std::vector<float> > dists;
            _p_index->radiusSearch( pos , indices, dists, range, flann::SearchParams(128));
            for(unsigned int i=0;i<indices.size();i++) {

            }
        }
    }

    ObjectiveNode* find_nearest( ObjectiveVector pos ) {
        if( _p_index ) {
            std::vector<std::vector<int> > indices;
            std::vector<std::vector<float> > dists;
            _p_index->knnSearch( pos , indices, dists, 1, flann::SearchParams(128));
        }
        return NULL;
    }

    flann::Index<flann::L2<float> >* _p_index;
};


#endif // OBJECTIVE_KNN_H_