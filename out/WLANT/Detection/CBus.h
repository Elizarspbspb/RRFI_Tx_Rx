#pragma once

#include "Includes.h"

//! Class for data interchange
template <class T> class CBus : private deque<T>
{
public:
	CBus() : deque<T>(){};
	~CBus(){};

	void pushEl(T elem)
	{
		//push_back(elem); было
		this->push_back(elem); //стало
	};

	T popEl()
	{
		//T tmp = front(); //было в винде
		T tmp = this->front(); //стало в линукс
		//pop_front(); //было
		this->pop_front(); //стало
		return (tmp);
	};

	T getFrontEl()
	{
		//return front(); //было в винде
		return this->front(); //стало
	};

	unsigned getSize()
	{
		//return size(); //было
		return this->size(); //сало
	};

	bool isEmpty()
	{
		//return empty(); //было
		return this->empty(); //стало
	};

	void pushFrontEl(T elem)
	{
		//push_front(elem);
		this->push_front(elem);
	};
};



