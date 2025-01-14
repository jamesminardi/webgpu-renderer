#pragma once

#include <string>

template <typename T, typename U>
class IObserver {
public:

	virtual void onNotify(T& source, const U& data) = 0;
	virtual ~IObserver() = default;
};

template <typename T, typename U>
class IObservable {
public:
	void addObserver(IObserver<T, U>& observer) {
		m_observers.push_back(&observer);
	}

	void removeObserver(IObserver<T, U>& observer) {
		m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), &observer), m_observers.end());
	}

	virtual void notify(const U& data) {
		for (auto& observer : m_observers) {
			observer->onNotify(*static_cast<T *>(this), data);
		}
	}
	virtual ~IObservable() = default;

private:
	std::vector<IObserver<T, U>*> m_observers;
};