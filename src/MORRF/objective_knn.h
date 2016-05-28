#ifndef OBJECTIVE_KNN_H_
#define OBJECTIVE_KNN_H_

#include <list>
#include <limits>
#include <flann/flann.hpp>

class RRTree;

class ObjectiveVector : public flann::Matrix<float>{
public:
    ObjectiveVector( int dimension, RRTree* p_tree = NULL) {
        _dimension = dimension;
        _value = new float[_dimension];
        for(unsigned int i=0; i<_dimension;i++) {
            _value[i] = std::numeric_limits<float>::max();
        }
        mp_tree = p_tree;
        flann::Matrix<float>(_value, 1, _dimension);
    }

    ObjectiveVector( float*  p_val, int dimension, RRTree* p_tree = NULL) {
        _dimension = dimension;
        _value = new float[_dimension];
        for(unsigned int i=0; i<_dimension;i++) {
            _value[i] = p_val[i];
        }
        mp_tree = p_tree;
        flann::Matrix<float>(_value, 1, _dimension);
    }

    ObjectiveVector( const ObjectiveVector & x ) {
        _dimension = x._dimension;
        if(_value) {
            delete _value;
            _value = new float[_dimension];
        }
        for(unsigned int i=0; i<_dimension;i++) {
            _value[i] = x._value[i];
        }
        flann::Matrix<float>(x.ptr(), 1, _dimension);
    }

    ~ObjectiveVector(){
        mp_tree = NULL;
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

    float operator[]( size_t const N ) const {
        return _value[N];
    }

    void set_value(int i, float val) {
        if(i<0 && i>=_dimension) {
            _value[i] = val;
        }
    }

    bool operator==(const ObjectiveVector &other) const {
        if(_dimension != other._dimension) {
            return false;
        }
        for (unsigned int i = 0; i < _dimension; i++) {
            if(_value[i]!=other._value[i]) {
                return false;
            }
        }
        return true;
    }

    int get_dimension() const {
        return _dimension;
    }

protected:
    float* _value;
    int    _dimension;
    RRTree* mp_tree;
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
    ObjectiveKNN(int K) {
        m_K = K;
        _p_index = NULL;
    }

    virtual ~ObjectiveKNN() {
        if(_p_index) {
            delete _p_index;
            _p_index = NULL;
        }
    }

    void insert( ObjectiveVector& vec ) {
        if( _p_index == NULL ) {
            _p_index = new flann::Index<flann::L2<float> >( vec, flann::KDTreeIndexParams(4) );
            //_p_index->buildIndex();
        }
        else {
            _p_index->addPoints( vec );
            //_p_index->buildIndex();
        }
    }

    size_t size() const {
        return _p_index->size();
    }

    void find_within_range( ObjectiveVector vec, float range, std::list<ObjectiveVector>& vec_list ) {
        if( _p_index ) {
            std::vector<std::vector<int> > indices;
            std::vector<std::vector<float> > dists;
            _p_index->radiusSearch( vec , indices, dists, range, flann::SearchParams(128));
            for(unsigned int i=0;i<indices.size();i++) {

            }
        }
    }

    ObjectiveVector* find_nearest( ObjectiveVector vec ) {
        if( _p_index ) {
            std::vector<std::vector<int> > indices;
            std::vector<std::vector<float> > dists;
            _p_index->knnSearch( vec , indices, dists, 1, flann::SearchParams(128));
            for(unsigned int i=0;i<indices.size();i++) {
                std::cout << indices[i][0] << std::endl;
            }
        }
        return NULL;
    }

    float get_sparse_diversity( ObjectiveVector& vec ) {
        float sparse_diversity = 0.0;
        if( _p_index ) {
            std::vector<std::vector<int> > indices;
            std::vector<std::vector<float> > dists;
            _p_index->knnSearch( vec , indices, dists, m_K, flann::SearchParams(128));

        }
        return sparse_diversity;
    }

    int m_K;
    flann::Index<flann::L2<float> >* _p_index;
};


#endif // OBJECTIVE_KNN_H_
