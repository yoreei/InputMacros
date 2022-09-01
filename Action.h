#ifndef _ACTION_
#define _ACTION_
#include "Utils.h"

#include <Windows.h>
#include <iostream>
enum CoroState { DONE, CONTINUE };
struct Action {
	virtual CoroState act() = 0;
	virtual void printDebug() = 0;
};

struct Delay : public Action {
	long long delay_ms;
	bool initialized = false;
	long long start_time;
	Delay(long long _delay_ms) : delay_ms(_delay_ms) {};
	virtual void printDebug()
	{
		std::cout << "Delay <" << delay_ms << "ms>" << std::endl;
	}
	virtual CoroState act()
	{
		if (initialized)
		{
			if (start_time + delay_ms > Utils::now())
			{
				return DONE;
			}
			else
			{
				return CONTINUE;
			}
		}
		else
		{
			initialized = true;
			start_time = Utils::now();
		}
	}
};

struct KeyPress : public Action {
	WORD vkCode;
	int direction;
	KeyPress(DWORD _vkCode, int _direction)
	{
		if (_vkCode < 255)
		{
			vkCode = (WORD)_vkCode;
		}
		else
		{
			std::cout << "Error _vkCode too large" << std::endl;
			exit(1);
		}
		switch (_direction)
		{
		case (WM_KEYDOWN):
			direction = 0;
			break;
		case (WM_KEYUP):
			direction = KEYEVENTF_KEYUP;
			break;
		default:
			std::cout << "Error direction code" << std::endl;
			exit(1);
		}
	}
	virtual void printDebug()
	{
		std::cout << "KeyPress <" << vkCode << ", " << direction << ">" << std::endl;
	}
	virtual CoroState act()
	{
		INPUT ip;

		// Set up a generic keyboard event.
		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0; // hardware scan code for key
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		// Press
		ip.ki.wVk = vkCode;
		ip.ki.dwFlags = direction;
		SendInput(1, &ip, sizeof(INPUT));
		return DONE;
	}
};
class ActionsSequence : private std::vector<std::unique_ptr<Action>> {
	using super = std::vector<std::unique_ptr<Action>>;
public:
	CoroState exec()
	{
		if (empty() || seq_iter == end())
		{
			std::cout << "Done playing sequence of length " << size() << std::endl;
			return DONE;
		}
		else {
			CoroState action_res = seq_iter->get()->act();
			advance(seq_iter, action_res == DONE);
			return CONTINUE;
		}
	}
	void reset()
	{
		seq_iter = begin();
	}
	void push_back (std::unique_ptr<Action> action)
	{
		super::push_back(std::move(action));
	}
	void clear ()
	{
		super::clear();
	}
	void print()
	{
		for (auto& action : *this)
		{
			action->printDebug();
		}
		std::cout << "-----------------" << std::endl << std::endl;
	}
private:
	super::iterator seq_iter = begin();
};

#endif // _ACTION_