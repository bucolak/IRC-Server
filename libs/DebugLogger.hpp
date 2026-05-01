#ifndef DEBUGLOGGER_HPP
#define DEBUGLOGGER_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <sys/time.h>

inline std::string debug_escape(const std::string &value) {
  std::string out;
  for (size_t i = 0; i < value.size(); ++i) {
    const char c = value[i];
    if (c == '\\' || c == '"')
      out += '\\';
    if (c == '\n')
      out += "\\n";
    else if (c == '\r')
      out += "\\r";
    else
      out += c;
  }
  return out;
}

inline long long debug_now_ms() {
  timeval tv;
  gettimeofday(&tv, 0);
  return static_cast<long long>(tv.tv_sec) * 1000LL + tv.tv_usec / 1000LL;
}

inline void debug_log(const std::string &location, const std::string &run_id,
                      const std::string &hypothesis_id,
                      const std::string &message, const std::string &data) {
  std::ofstream file(
      "/home/aaydogdu/Desktop/ft_irc/.cursor/debug-64cbe2.log",
      std::ios::app);
  if (!file.is_open())
    return;
  file << "{\"sessionId\":\"64cbe2\",\"runId\":\"" << debug_escape(run_id)
       << "\",\"hypothesisId\":\"" << debug_escape(hypothesis_id)
       << "\",\"location\":\"" << debug_escape(location) << "\",\"message\":\""
       << debug_escape(message) << "\",\"data\":{\"details\":\""
       << debug_escape(data) << "\"},\"timestamp\":" << debug_now_ms() << "}\n";
}

#endif
