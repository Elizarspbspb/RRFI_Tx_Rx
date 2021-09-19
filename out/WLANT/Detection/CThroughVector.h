#pragma once

#include "Includes.h"

template <class T> class CThroughVector : public vector<T>
{
public:
	CThroughVector(unsigned uSize) : vector<T>() { m_uCurrentSize = 0; m_uSize = uSize; isFix = false; };
	CThroughVector() : vector<T>() { m_uCurrentSize = 0; m_uSize = 1; isFix = false; };

	virtual ~CThroughVector(void) {};

//	void setSize(unsigned uSize) { m_uCurrentSize = 0; m_uSize = uSize; clear(); isFix = false; };
	void setSize(unsigned uSize) { m_uCurrentSize = 0; m_uSize = uSize; clear; isFix = false; }; // стало

	unsigned getCurrentSize() { return m_uCurrentSize; };

	void addElement(T newElement)
	{
		//push_back(newElement); //ьбыло
		this->push_back(newElement); //стало

		if(isFix)
		{
            //erase(begin()); //ьыло
            //erase(this->begin()); //стало слишком много аргументов в фу-ии erase
		}

		if(!isFix)
			m_uCurrentSize++;

		if(m_uCurrentSize == m_uSize)
			isFix = true;
	};

	T getFirstElement()
	{
		//return front(); //было в винде
		return this->front(); //стало
	};

private:
	unsigned m_uSize;
	unsigned m_uCurrentSize;

	bool isFix;
};
