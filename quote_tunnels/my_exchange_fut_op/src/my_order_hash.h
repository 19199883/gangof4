/*
* my_order_hash
*
* Order hash is designed to store data associated with order, which retrieve by serial_no.
* rule of serial_no:
* 1. 1 000 001 315 : 315 is st_id, 000 001 is serial_no of the order, it's a normal order.
* 2. 2(>1) 000 001 315 : 315 is st_id, 000 001 is serial_no of the order, it's a transformed order of order above.
* 
* Author : Joseph
* Copyright(C) by MY Capital Inc. 2007-2015
*/

#ifndef MY_ORDER_HASH_H
#define MY_ORDER_HASH_H

#include <stdlib.h>
#include <list>
#include <vector>
#include <stdint.h>
#include <algorithm>

using namespace std;

template <class DataType>
class MyOrderHash
{
public:
    MyOrderHash(const uint64_t slot_cnt);
    ~MyOrderHash();

public:
    int AddData(const uint64_t serial_no, const DataType& data);
    int AddData(const uint64_t serial_no, const DataType* data);

    int GetData(const uint64_t serial_no, DataType& data);
    DataType* GetData(const uint64_t serial_no);

    void DelData(const uint64_t serial_no);

private:
    uint64_t m_SlotCnt;
    vector<list<pair<uint64_t, DataType>>> m_SlotVec;
};

template <class DataType>
MyOrderHash<DataType>::MyOrderHash(const uint64_t slot_cnt)
{
    m_SlotCnt = (slot_cnt < 1000000) ? 1000000 : slot_cnt;
    m_SlotVec.resize((unsigned int)m_SlotCnt);
}

template <class DataType>
MyOrderHash<DataType>::~MyOrderHash(void)
{
}

static const uint64_t 
_get_order_index(const uint64_t serial_no)
{
    //return (serial_no % 1000000000) / 1000;
    // modified at 20160321: last 8 digits used for model id (ABCDxxxx)
	// support c-trader, modified by wangying on 20170905
    return serial_no / 1000;
}

template <class DataType>
int
MyOrderHash<DataType>::AddData(const uint64_t serial_no, const DataType& data)
{
    uint64_t idx;
    pair<uint64_t, DataType> new_data;
    idx = _get_order_index(serial_no);

    list<pair<uint64_t, DataType>>& ord_list = m_SlotVec[idx];

#ifdef _DEBUG
    list<pair<uint64_t, DataType>>::iterator it = ord_list.begin();
    list<pair<uint64_t, DataType>>::iterator it_end = ord_list.end();

    while (it != it_end)
    {
        if (it->first == serial_no)
        {
            return -1;
        }

        it++;
    }
#endif

    new_data.first = serial_no;
    new_data.second = data;

    ord_list.push_back(new_data);

    return 0;
}

template <class DataType>
int
MyOrderHash<DataType>::AddData(const uint64_t serial_no, const DataType* data)
{
    uint64_t idx;
    pair<uint64_t, DataType> new_data;
    idx = _get_order_index(serial_no);

    list<pair<uint64_t, DataType>>& ord_list = m_SlotVec[idx];

#ifdef _DEBUG
    list<pair<uint64_t, DataType>>::iterator it = ord_list.begin();
    list<pair<uint64_t, DataType>>::iterator it_end = ord_list.end();

    while (it != it_end)
    {
        if (it->first == serial_no)
        {
            return -1;
        }

        it++;
    }
#endif

    new_data.first = serial_no;
    memcpy(&new_data.second, data, sizeof(DataType));

    ord_list.push_back(new_data);

    return 0;
}

template <class DataType>
int
MyOrderHash<DataType>::GetData(const uint64_t serial_no, DataType& data)
{
    typedef typename list<pair<uint64_t, DataType>>::iterator list_node_ite_t;
    uint64_t idx;
    pair<uint64_t, DataType> new_data;
    list<pair<uint64_t, DataType>>& ord_list = m_SlotVec[_get_order_index(serial_no)];
    list_node_ite_t it = ord_list.begin();
    
    while (it != ord_list.end())
    {
        if (it->first == serial_no)
        {
            memcpy(&data, &it->second, sizeof(data));
            return 0;
        }

        it++;
    }

    return -1;
}

template <class DataType>
DataType*
MyOrderHash<DataType>::GetData(const uint64_t serial_no)
{
    typedef typename list<pair<uint64_t, DataType>>::iterator list_node_ite_t;
    uint64_t idx;
    pair<uint64_t, DataType> new_data;
    idx = _get_order_index(serial_no);

    list<pair<uint64_t, DataType>>& ord_list = m_SlotVec[idx];

    list_node_ite_t it = ord_list.begin();
    list_node_ite_t it_end = ord_list.end();

    while (it != it_end)
    {
        if (it->first == serial_no)
        {
            return &it->second;
        }

        it++;
    }

    return NULL;
}

template <class DataType>
void
MyOrderHash<DataType>::DelData(const uint64_t serial_no)
{
    typedef typename list<pair<uint64_t, DataType>>::iterator list_node_ite_t;
    uint64_t idx;
    pair<uint64_t, DataType> new_data;
    idx = _get_order_index(serial_no);

    list<pair<uint64_t, DataType>>& ord_list = m_SlotVec[idx];

    list_node_ite_t it = ord_list.begin();
    list_node_ite_t it_end = ord_list.end();

    while (it != it_end)
    {
        if (it->first == serial_no)
        {
            ord_list.erase(it);
            return;
        }

        it++;
    }
}

#endif
