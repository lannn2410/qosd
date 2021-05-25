#ifndef _HEAP_DATA_H_
#define _HEAP_DATA_H_

template<class T1>
struct DescendingOrder{
	T1 *v1;
public:
	DescendingOrder(T1 *u1):v1(u1){};
	
	bool operator() (int &i, int &j) const{
		return v1[i] < v1[j];
	}
};

template<class T1>
struct AscendingOrder {
	T1 *v1;
public:
	AscendingOrder(T1 *u1) :v1(u1) {};

	bool operator() (int &i, int &j) const {
		return v1[i] > v1[j];
	}
};

#endif
