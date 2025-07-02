#ifndef REDBROUK_UTILS1_H
#define REDBROUK_UTILS1_H

#include <iostream>
#include <cstddef>
#include <cstdio>

// #define container_of(ptr, T, member) \
    // ((T *)( (char *)ptr - offsetof(T, member) ))

namespace redbrouk::utils
{

using std::byte;
using pdiff_t = std::ptrdiff_t;

template <typename T, typename M>
constexpr inline pdiff_t offset_of(M T::*member)
{
    return (pdiff_t)(&(((T *)(NULL))->*member));
}
template <typename T, typename MType>
constexpr inline T *container_of(MType *ptr, MType T::*mem)
{
	return (T *)((byte *)ptr - offset_of<T>(mem));
}


#define LOGGING_ON true
// Check if compiling with c or c++
#ifdef __cplusplus
// Check if c++ version is 2X
#if __cplusplus < 202002L
constexpr inline void log_impl(const char *file, int line, const char *func, auto &&msg_clsr) {
	std::cerr << "[" << file << ":" << line << " | " << func << "] " <<
		msg_clsr() << '\n';
	std::cerr.flush();
}
#else
template <typename T>
concept stm_printable = requires(T t) { // if the stream operator into std::cerr has been defined
	{ std::cerr << t };
};
inline void log_impl(const char *time, const char *file, int line, const char *func, stm_printable auto &&msg) {
	std::cerr << "[" << time << "] [" << file << ":" << line << " | " << func << "] " << msg << '\n';
	std::cerr.flush();
}
inline void log_impl(const char *time, const char *file, int line, const char *func, auto &&msg_clsr) {
	std::cerr << "[" << time << "] [" << file << ":" << line << " | " << func << "] " <<
		msg_clsr() << '\n';
	std::cerr.flush();
}
#endif // if __cplusplus < 202002L
	#define LOG(x) do { \
		if constexpr (LOGGING_ON) \
		{ \
			time_t rawtime; \
			struct tm * timeinfo; \
			char timestamp[20]; \
			time(&rawtime); \
			timeinfo = localtime(&rawtime); \
			strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo); \
			utils::log_impl(timestamp, __FILE__, __LINE__, __func__, x); \
		} \
	} while(0);

#elif LOGGING_ON // ifdef __cplusplus (if using c, not c++ and logging is on)
	#define LOG(x) do { \
		time_t rawtime; \
		struct tm * timeinfo; \
		char timestamp[20]; \
		time(&rawtime); \
		timeinfo = localtime(&rawtime); \
		strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo); \
		fprintf(stderr, "[%s] [%s:%d %s()] ", timestamp, __FILE__, __LINE__, __func__); \
		fprintf(stderr, "%s\n", x); \
		fflush(stderr);
	} while(0)
#else // if logging is off
#define LOG(X)

#endif // ifdef __cplusplus
//

#if __cplusplus >= 202002L

#include <format>
template<class... Args>
std::string fmt(std::format_string<Args...> s, Args&&... args) {
    return std::format(s, std::forward<Args>(args)...);
}

#define MAKE_FORMATTER(T, FUN) \
template<> \
struct std::formatter<T> : std::formatter<std::string> { \
	auto format(const T &type, std::format_context &ctx) const { \
		std::string str; \
		FUN \
		return std::formatter<std::string>::format(std::format("{}", str), ctx); \
	} \
};

#define MAKE_FORMATTER_T(T, FUN, ...) \
template<__VA_ARGS__> \
struct std::formatter<T> : std::formatter<std::string> { \
	auto format(const T &type, std::format_context &ctx) const { \
		std::string str; \
		FUN \
		return std::formatter<std::string>::format(std::format("{}", str), ctx); \
	} \
};

#endif

} // namespace redbrouk::utils

#endif
