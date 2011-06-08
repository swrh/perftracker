#if !defined(_PERFTRACKER_HPP_)
#define _PERFTRACKER_HPP_

#include <math.h>
#include <string.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <map>
#include <list>

#define PT_DEFAULTFILENAME ("perftracker.out")
#define PT_TRACKPOINT() perftracker::auto_track __trackpoint(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#define PT_SETFILENAME(x) (perftracker::tracker::get_instance().set_filename(x))
#define PT_DUMP() (perftracker::tracker::get_instance().dump())

namespace
perftracker
{

class
timer
{
public:
	typedef struct timeval time_type;

	timer()
	{
		running = false;
		t_start = t_split = t_lastsplit = t_stop = get_zero();
	}

	~timer()
	{
	}

	time_type
	now() const
	{
		time_type tv;
		struct timezone tz;

		::gettimeofday(&tv, &tz);

		return tv;
	}

	double
	diff(const time_type &a, const time_type &b) const
	{
		return static_cast<double>(b.tv_sec - a.tv_sec) + static_cast<double>(b.tv_usec - a.tv_usec) / 1e6;
	}

	double
	start()
	{
		if (is_running())
			return NAN;

		t_start = t_split = t_lastsplit = t_stop = now();
		running = true;

		return t_start.tv_sec + (static_cast<double>(t_start.tv_usec) / 1e6);
	}

	double
	split()
	{
		t_stop = now();
		t_lastsplit = t_split;
		t_split = t_stop;

		return diff(t_split, t_stop);
	}

	double
	current() const
	{
		return diff(t_split, now());
	}

	double
	lastsplit() const
	{
		return diff(t_lastsplit, t_split);
	}

	double
	stop()
	{
		t_stop = now();
		running = false;

		return total();
	}

	double
	total()
	{
		if (is_running())
			t_stop = now();

		return diff(t_start, t_stop);
	}

	inline bool
	is_running() const
	{
		return running;
	}

private:
	time_type
	get_zero()
	{
		time_type z;

		z.tv_sec = 0;
		z.tv_usec = 0;

		return z;
	}

private:
	time_type t_start, t_split, t_lastsplit, t_stop;
	bool running;

};

struct
track_point_st
{
	char file[256];
	unsigned int line;
	char function[256];
	unsigned int size;
};

class
track_point
{
public:
	track_point(const char *file_, unsigned int line_, const char *func_)
		: file(file_), line(line_), function(func_)
	{
	}

	bool
	operator <(const track_point &p) const
	{
		if (function != p.function)
			return function < p.function;
		if (file != p.file)
			return file < p.file;
		return line < p.line;
	}

public:
	const char *
	get_file() const
	{
		return file;
	}

	unsigned int
	get_line() const
	{
		return line;
	}

	const char *
	get_function() const
	{
		return function;
	}

	void
	get_struct(struct track_point_st *st, unsigned int size) const
	{
		::bzero(st, sizeof(*st));

		::snprintf(st->file, sizeof(st->file), "%s", get_file());
		st->line = get_line();
		::snprintf(st->function, sizeof(st->function), "%s", get_function());
		st->size = size;
	}

private:
	const char *file;
	unsigned int line;
	const char *function;

};

struct
time_entry
{
	time_entry(double when_, double total_, uint64_t ncalls_ = 1)
		: when(when_), total(total_), ncalls(ncalls_)
	{
	}

	double when, total;
	uint64_t ncalls;
};

class
entry_handler
{
public:
	entry_handler()
	{
	}

	~entry_handler()
	{
	}

	std::list<time_entry> &
	get_entries()
	{
		return real_entries;
	}

private:
	time_entry
	average(const std::list<time_entry> es)
	{
		time_entry entry(0, 0, 0);

		for (std::list<time_entry>::const_iterator it = es.begin(); it != es.end(); it++)
			entry.total += it->total;

		entry.ncalls = es.size();
		entry.when = es.rbegin()->when + (es.begin()->when - es.rbegin()->when) / entry.ncalls;
		entry.total /= entry.ncalls;

		return entry;
	}

	void
	consolidate()
	{
		time_entry &r = *real_entries.begin();
		time_entry t = *temp_entries.begin();

		if (t.when - r.when <= 1)
			return;

		if (temp_entries.size() > 1) {
			temp_entries.pop_front();
			temp_entries.push_back(r);
			r = average(temp_entries);
		}

		real_entries.push_front(t);
		temp_entries.clear();
	}

public:
	void
	push(const time_entry &e)
	{
		if (real_entries.size() == 0) {
			real_entries.push_front(e);
			return;
		}

		if (temp_entries.size() > 0) {
			temp_entries.push_front(e);
			consolidate();
			return;
		}

		if (e.when - real_entries.begin()->when < 1) {
			temp_entries.push_front(e);
			return;
		}

		real_entries.push_front(e);
	}

private:
	std::list<time_entry> real_entries, temp_entries;
};

template <typename Lockable>
class scope_locker
{
public:
	scope_locker(Lockable &lockable_)
		: lockable(lockable_)
	{
		lockable.lock();
	}

	~scope_locker()
	{
		lockable.unlock();
	}

private:
	Lockable &lockable;

};

class
tracker
{
private:
	tracker()
	{
		pthread_mutex_init(&mutex, NULL);
		set_filename(PT_DEFAULTFILENAME);
	}

public:
	~tracker()
	{
		dump();
		pthread_mutex_destroy(&mutex);
	}

private:
	pthread_mutex_t mutex;

public:
	void
	lock()
	{
		pthread_mutex_lock(&mutex);
	}

	void
	unlock()
	{
		pthread_mutex_unlock(&mutex);
	}

public:
	void
	dump()
	{
		if (get_filename().size() != 0) {
			std::ofstream out(get_filename().c_str(), std::ios::binary);
			dump_binary(out);
		} else {
			dump_human(std::cout);
		}
	}

	void
	dump_binary(std::ostream &out)
	{
		scope_locker<tracker> locker(*this);
		for (std::map<track_point, entry_handler>::iterator it = log.begin(); it != log.end(); it++) {
			std::list<time_entry> &l = it->second.get_entries();
			struct track_point_st p;
			it->first.get_struct(&p, l.size());
			unsigned long long int sz = 0;
			for (std::list<time_entry>::iterator e = l.begin(); e != l.end(); e++)
				sz++;
			//std::cout << p.file << ":" << p.line << ": " << p.function << "; (" << sz << " vs " << l.size() << ")" << std::endl;
			out.write(reinterpret_cast<const char *>(&p), sizeof(p));
			for (std::list<time_entry>::reverse_iterator e = l.rbegin(); e != l.rend(); e++)
				out.write(reinterpret_cast<const char *>(&*e), sizeof(struct time_entry));
		}
	}

	void
	dump_human(std::ostream &out)
	{
		scope_locker<tracker> locker(*this);
		for (std::map<track_point, entry_handler>::iterator it = log.begin(); it != log.end(); it++) {
			const track_point &p = it->first;
			std::list<time_entry> &l = it->second.get_entries();
			out << "" << p.get_file() << ":" << p.get_line() << ": " << p.get_function() << std::endl;
			for (std::list<time_entry>::reverse_iterator e = l.rbegin(); e != l.rend(); e++)
				out << "  " << e->when << ": " << e->total << " secs;" << std::endl;
		}
	}

public:
	void
	push(track_point point, time_entry tim)
	{
		scope_locker<tracker> locker(*this);
		log[point].push(tim);
	}

private:
	std::map<track_point, entry_handler> log;

public:
	static tracker &
	get_instance()
	{
		return instance;
	}

private:
	static tracker instance;

public:
	void
	set_filename(const char *filename_)
	{
		if (filename_ == NULL)
			filename_ = "";
		filename = filename_;
	}

	const std::string &
	get_filename() const
	{
		return filename;
	}

private:
	std::string filename;

};

class
auto_track
{
public:
	inline
	auto_track(const char *file, unsigned int line, const char *function = NULL)
		: point(file, line, function)
	{
		start = t.start();
	}

	inline
	~auto_track()
	{
		time_entry tim(start, t.stop());
		tracker::get_instance().push(point, tim);
	}

private:
	timer t;
	track_point point;
	double start;

};

}

#endif // !defined(_PERFTRACKER_HPP_)
