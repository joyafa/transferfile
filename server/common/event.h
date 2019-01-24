#pragma once
#include <string>
class iEvent
{
public:
	iEvent(uint32_t eid, uint32_t sn);
	virtual ~iEvent();
    virtual std::ostream& dump(std::ostream& out) const { return out; }; 
    uint32_t generateSeqNo();
    uint32_t get_eid() const { return eid_; };
    uint32_t get_sn() const { return sn_; };
    void set_eid(uint32_t eid){ eid_ = eid; };
private:
    uint32_t   eid_;    /* 事件ID */
    uint32_t   sn_;     /* 事件的序列号 */
};
