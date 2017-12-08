#pragma once

#include <queue>
#include <vector>
#include <string>

#include <thread>
#include <mutex>
#include <condition_variable>
 
#include <fmt/printf.h>

namespace Lettvin
{

using namespace std;

//______________________________________________________________________________
// https://juanchopanzacpp.wordpress.com/2013/02/26/concurrent-queue-c11/
// modified from original by trivial renaming of members
template <typename T>
class ThreadedQueue
{

//------
public:
//------

	ThreadedQueue (size_t a_maximum=64) : m_maximum (a_maximum) {}

	//--------------------------------------------------------------------------
	T pop ()
	{
		std::unique_lock<std::mutex> mlock (m_mutex);
		while (m_queue.empty ())
		{
			m_available.wait (mlock);
		}
		auto item = m_queue.front ();
		m_queue.pop ();

		// Needs confirmation
		m_full.notify_one ();

		return item;
	} // pop ()

	//--------------------------------------------------------------------------
	void pop (T& a_item)
	{
		std::unique_lock<std::mutex> mlock (m_mutex);
		while (m_queue.empty ())
		{
			m_available.wait (mlock);
		}
		a_item = m_queue.front ();
		m_queue.pop ();

		// Needs confirmation
		m_full.notify_one ();

	} // pop (item)

	//--------------------------------------------------------------------------
	void push (const T& a_item)
	{
		std::unique_lock<std::mutex> mlock (m_mutex);

		// Needs confirmation
		while (m_queue.size () == m_maximum)
		{
			m_full.wait (mlock);
		}

		m_queue.push (a_item);
		mlock.unlock ();
		m_available.notify_one ();
	} // push (item) const

	//--------------------------------------------------------------------------
	void push (T&& a_item)
	{
		std::unique_lock<std::mutex> mlock (m_mutex);

		// Needs confirmation
		while (m_queue.size () == m_maximum)
		{
			m_full.wait (mlock);
		}

		m_queue.push (std::move (a_item));
		mlock.unlock ();
		m_available.notify_one ();
	} // push (item) move

//------
private:
//------

	std::queue<T> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_available;
	std::condition_variable m_full;       // TODO prevent more than max
	size_t m_maximum{64};                 // TODO prevent more than max

}; // class ThreadedQueue

#ifdef MAIN
//______________________________________________________________________________
/// Demonstration use of ThreadedQueue
class Distribute
{

//------
public:
//------

	//--------------------------------------------------------------------------
	Distribute (size_t a_max=64) : m_cpus (a_max), m_queue (a_max) {}

	//--------------------------------------------------------------------------
	void operator ()(vector<string>& a_tokens)
	{
		// One less workers than cpus
		for (size_t thid=1; thid < m_cpus; ++thid)
		{
			m_threads.emplace_back (
				thread{
					[&] ()
					{
						string tgt;
						// Note how empty string terminates loop
						while ((tgt = m_queue.pop ()).size () != 0)
						{
							fmt::printf ("%s\n", tgt.c_str ());
						}
					}
				}
			);
		}
		// One master worker for the last cpu
		m_threads.emplace_back (
			thread
			{
				[&] ()
				{
					for (auto token:a_tokens)
					{
						m_queue.push (token);
					}
					// cleanup: terminate worker threads with empty string args
					while (m_cpus--)
					{
						m_queue.push ("");
					}
				}
			}
		);
		for (auto& thrd:m_threads) thrd.join ();
	} // operator ()()

//------
private:
//------

	size_t                m_cpus;
	vector<thread>        m_threads;
	ThreadedQueue<string> m_queue;

}; // class Distribute
#endif

}  // namespace Lettvin
