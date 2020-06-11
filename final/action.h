#pragma once
#include <algorithm>
#include <unordered_map>
#include <string>
#include "board.h"

class Action {
public:
	Action(unsigned code = -1u, int color = -1) : code(code), color(color) {}
	Action(const Action& a) : code(a.code), color(a.color) {}
	virtual ~Action() {}

	class Eat; 	// create a eating action
	class Move; // create a moving action

public:
	virtual int apply(Board& b) const {
		auto proto = entries().find(type());
		if (proto != entries().end()) return proto->second->reinterpret(this).apply(b);
		return -1;
	}
	virtual std::ostream& operator >>(std::ostream& out) const {
        auto proto = entries().find(type());
        if (proto != entries().end()) return proto->second->reinterpret(this) >> out;
        return out << "??";
    }
    virtual std::istream& operator <<(std::istream& in) {
        auto state = in.rdstate();
        for (auto proto = entries().begin(); proto != entries().end(); proto++) {
            if (proto->second->reinterpret(this) << in) return in;
            in.clear(state);
        }
        return in.ignore(2);
    }

public:
	operator unsigned() const { return code; }
	unsigned type() const { return code & type_flag(-1u); }
	unsigned destination() const { return code & 0b111111; }

protected:
	static constexpr unsigned type_flag(unsigned v) { return v << 24; }

	typedef std::unordered_map<unsigned, Action*> prototype;
	static prototype& entries() { static prototype m; return m; }
	virtual Action& reinterpret(const Action* a) const { return *new (const_cast<Action*>(a)) Action(*a); }

	unsigned code;
    int color;
};

class Action::Eat : public Action {
public:
	static constexpr unsigned type = type_flag('e');
	Eat(unsigned code, int color) : Action(Eat::type | code, color) {}
	Eat(const Action& a = {}) : Action(a) {}

public:
	int apply(Board& b) const {
		return b.eat(destination(), color);
	}

protected:
	Action& reinterpret(const Action* a) const { return *new (const_cast<Action*>(a)) Eat(*a); }
	static __attribute__((constructor)) void init() { entries()[type_flag('e')] = new Eat; }
};

class Action::Move : public Action {
public:
	static constexpr unsigned type = type_flag('m');
	Move(unsigned code, int color) : Action(Move::type | code, color) {}
	Move(const Action& a = {}) : Action(a) {}

public:
	int apply(Board& b) const {
		return b.move(destination(), color);
	}

protected:
	Action& reinterpret(const Action* a) const { return *new (const_cast<Action*>(a)) Move(*a); }
	static __attribute__((constructor)) void init() { entries()[type_flag('m')] = new Move; }
};