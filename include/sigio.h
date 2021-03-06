// C++ STL headers
#include <csignal>
#include <chrono>
#include <ratio>
#include <memory>
#include <functional>
#include <map>
#include <system_error>

// System headers
#include <sys/inotify.h>

#pragma once

using namespace std::chrono;
using namespace std::chrono_literals;

namespace one {
  /**
   * C++17 wrapper for signal driven I/O.
   */
  class sigio {

    class sigio_;                             /**< Forward class reference to PIMPL implementation */
    sigio_* _sigio;                           /**< pointer to PIMPL timer_ object */

    //TODO: create a real constructor
    sigio();


    public:
    using callback_t = std::function<void(siginfo_t*)>; /**< User provided callback function type*/
    using inotify_callback_t = std::function<void(struct inotify_event*)>;

    /**
     * Defines all error codes for the timer class implementation
     */
    enum class error : int
    {
      // critical errors, decrease negative number to add a new error
      sigio_object_creation = -254,           /**< create function call has failed */
      signal_handler_registration,            /**< System signal handler registration has failed */
      activate_already_active,                /**< An attempt to activate file descriptor which is already activated*/
      deactivate_not_active,                  /**< An attempt to deactivate file descriptor which is not activated*/
      sig_handler_fd_unknown,                 /**< The file descriptor passed by system to signal hanler is unknown*/
      unknown_error = -1,                     /**< Non identified system error */

      // no errors, successful operation
      success = 0,                            /**< No errors found, operation has completed successfully*/

      // warnings, increase number to add a new warning
      signal_handler_null_pointer,           /**< Signal handler receives null pointer to _sigio object */
      signal_handler_unexpected_signal       /**< Signal handler receives unexpected signal ID */
    };

    /**
     * struct timer::error_category child  of st::error_category
     * It overrides and implements name and message.
     */
    struct error_category : std::error_category
    {
      /**
       * Overriden method return domain name for the given error_category struct
       *
       * @return  always return const char pointer to the name string
       */
      const char* name() const noexcept override
      {
        return "sigio";
      }

      /**
       * Overriden method return error mesage string for the given error from timer::error enum
       */
      std::string message(int err) const override
      {
        static std::map<int, std::string> err2str =
        {
          // errors
          {static_cast<int>(error::sigio_object_creation),
            "Creation of sigio object has failed"},
          {static_cast<int>(error::signal_handler_registration),
            "SYSTEM sigaction registration has failed"},
          {static_cast<int>(error::activate_already_active),
            "An attempt to activate file descriptor which is already active"},
          {static_cast<int>(error::deactivate_not_active),
            "An attempt to deactivate non-active file descriptor"},
          {static_cast<int>(error::sig_handler_fd_unknown),
            "The file descriptor passsed by system to signal handler is unknown"},
          {static_cast<int>(error::unknown_error),
            "unknown error"},

          // warnings
          {static_cast<int>(error::signal_handler_null_pointer),
            "Signal handler receives null pointer to _sigio object"},
          {static_cast<int>(error::signal_handler_unexpected_signal),
            "Signal handler receives unexpected signal ID"}
        };

        if (!err2str.count(err))
        {
          return std::string("Unknown error");
        }

        return err2str[err];
      }

      static error_category& instance()
      {
        static error_category instance;
        return instance;
      }
    }; // struct error_category

    virtual ~sigio();

    sigio(const sigio&) = delete;
    sigio(sigio&&) = delete;
    sigio& operator=(const sigio&) = delete;
    sigio& operator=(sigio&&) = delete;

    inline static sigio& get()
    {
      static sigio instance;
      return instance;
    }

    int get_signal() const;
    std::error_code set_signal(int signal);

    void activate(int fd, callback_t cb,
        seconds timeout_sec = 0s,
        nanoseconds timeout_nsec = 0ns);

    void activate(const std::string fn, uint32_t inotify_mask, inotify_callback_t cb,
          seconds timeout_sec = 0s,
          nanoseconds timeout_nsec = 0ns);

    void deactivate(int fd);
    void deactivate(const std::string fn);

    bool is_activated(int fd) const noexcept;
    bool is_activated(const std::string fn) const noexcept;

    std::error_code try_activate(int fd, callback_t cb, seconds timeout_sec = 0s,
                                 nanoseconds timeout_nsec = 0ns) noexcept;
    std::error_code try_deactivate(int fd) noexcept;

    std::error_code try_activate(const std::string fn, uint32_t inotify_mask,  inotify_callback_t cb,
                                 seconds timeout_sec = 0s, nanoseconds timeout_nsec = 0ns) noexcept;
    std::error_code try_deactivate(const std::string fn) noexcept;
  }; // class sigio

  inline std::error_code make_error_code(sigio::error err) noexcept
  {
    return std::error_code(static_cast<int>(err), sigio::error_category::instance());
  }

  inline bool is_warning(std::error_code ec)
  {
    return (ec && ec.value() > 0);
  }

}// namespace one

namespace std
{
  template <>
    struct is_error_code_enum<::one::sigio::error> : true_type {};
} // namespace std
