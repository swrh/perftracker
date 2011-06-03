#if !defined(_PERFTRACKER_HPP_)
#define _PERFTRACKER_HPP_

#include <err.h>
#include <errno.h>

#include <string.h>
#include <sys/time.h>

#include <map>
#include <list>

#define _ENOSYS() do { errno = ENOSYS; err(EXIT_FAILURE, "%s:%d", __FILE__, __LINE__); } while (0)
#define PERF_TRACK() perftracker::auto_track __autotrack(__FILE__, __LINE__, __PRETTY_FUNCTION__)

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
		return double(b.tv_sec - a.tv_sec) + double(b.tv_usec - a.tv_usec) / 1e6;
	}

	bool
	start()
	{
		if (is_running())
			return false;

		t_start = t_split = t_lastsplit = t_stop = now();
		running = true;

		return true;
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

class
track_point
{
public:
	track_point(const char *file_, unsigned int line_)
		: file(file_), line(line_)
	{
	}

	bool
	operator ==(const track_point &p) const
	{
		if (line != p.line)
			return false;
		if (file != p.file && ::strcmp(file, p.file) != 0)
			return false;

		return true;
	}

	bool
	operator <(const track_point &p) const
	{
		if (file == p.file)
			return line < p.line;
		return file < p.file;
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

private:
	const char *file;
	unsigned int line;

};

struct
time_entry
{
	time_entry(double tim_, const char *func_)
		: tim(tim_), func(func_)
	{
	}

	double tim;
	const char *func;
};

class
tracker
{
private:
	tracker()
	{
	}

public:
	~tracker()
	{
		for (std::map<track_point, std::list<struct time_entry> >::iterator it = log.begin(); it != log.end(); it++) {
			std::cout << "- " << it->first.get_file() << ":" << it->first.get_line() << ":" << std::endl;
			for (std::list<struct time_entry>::iterator e = it->second.begin(); e != it->second.end(); e++) {
				std::cout << "  - " << e->tim << " secs -- " << e->func << ";" << std::endl;
			}
		}
	}

public:
	void
	push(track_point point, double tim, const char *func)
	{
		log[point].push_back(time_entry(tim, func));
	}

private:
	std::map<track_point, std::list<struct time_entry> > log;

public:
	static tracker &
	get_instance()
	{
		return instance;
	}

private:
	static tracker instance;

};

class
auto_track
{
public:
	auto_track(const char *file, unsigned int line, const char *func_ = NULL)
		: point(file, line), func(func_)
	{
		t.start();
	}

	~auto_track()
	{
		double tim = t.stop();

		tracker::get_instance().push(point, tim, func);
	}

private:
	timer t;
	track_point point;
	const char *func;

};

}

#endif // !defined(_PERFTRACKER_HPP_)
