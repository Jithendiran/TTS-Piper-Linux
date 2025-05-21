#include "include.hpp"

class IProcess_c
{
private:
    int pid = -1;
    bool is_async = false;

    virtual bool set_io_flag(int &fd, int flag)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
        {
            perror("fcntl");
            return false;
        }
        if (flags & flag)
            return true;
        if (fcntl(fd, F_SETFL, flags | flag) == -1)
        {
            perror("fcntl");
            return false;
        }
        flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
        {
            perror("fcntl");
            return false;
        }
        if (flags & flag)
            return true;
        return false;
    }

    virtual bool clear_io_flag(int &fd, int flag)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
        {
            perror("fcntl");
            return false;
        }
        if (!(flags & flag))
            return true;

        flags &= ~flag; // Clear the  bit

        if (fcntl(fd, F_SETFL, flags) == -1)
        {
            perror("fcntl");
            return false;
        }
        flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
        {
            perror("fcntl");
            return false;
        }
        if (!(flags & flag))
            return true;
        return false;
    }

protected:
    int ip_pipe[2];
    int op_pipe[2];
    int err_pipe[2];

public:
    bool close_pipe(int &pipe_)
    {
        if (pipe_ >= 1)
        {
            if (close(pipe_) == -1)
                return false;
            pipe_ = -1;
        }
        return true;
    }

    bool close_ip()
    {
        return close_pipe(ip_pipe[1]);
    }
    bool close_op()
    {
        return close_pipe(op_pipe[0]);
    }
    bool close_err()
    {
        return close_pipe(err_pipe[0]);
    }

    bool can_read_pipe(int pipe_)
    {
        if (pipe_ > 0)
        {
            fd_set read_fds;
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            FD_ZERO(&read_fds);
            FD_SET(pipe_, &read_fds);
            return select(pipe_ + 1, &read_fds, NULL, NULL, &timeout) > 0 && FD_ISSET(pipe_, &read_fds);
        }
        return false;
    }

    bool can_write_pipe(int pipe_)
    {
        if (pipe_ > 0)
        {
            fd_set fds;
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            FD_ZERO(&fds);
            FD_SET(pipe_, &fds);
            return select(pipe_ + 1, NULL, &fds, NULL, &timeout) > 0 && FD_ISSET(pipe_, &fds);
        }
        return false;
    }

    bool check_is_process_alive()
    {
        // Check if the process is alive
        if (pid > 0)
        {
            if (kill(pid, 0) == -1)
            {
                if (errno == ESRCH)
                {
                    cout << "Process does not exist." << endl;
                }
                else if (errno == EPERM)
                {
                    cout << "No permission to send signal to the process." << endl;
                }
                return false;
            }
            return true;
        }
        else
        {
            cout << "Invalid PID. Process is not running." << endl;
            return false;
        }
    }

    int getPid() { return pid; }

    virtual void setPid(int pid) { this->pid = pid; }

    // This function will pause the process
    virtual bool pause()
    {
        if (check_is_process_alive())
        {
            if (-1 == kill(pid, SIGSTOP))
            {
                return false;
            }
            return true;
        }
        return false;
    }

    // This function will resume the process
    virtual bool play()
    {
        if (check_is_process_alive())
        {
            if (-1 == kill(pid, SIGCONT))
            {
                return false;
            }
            return true;
        }
        return false;
    }

    // This function will terminate the process
    virtual bool stop()
    {
        if (check_is_process_alive())
        {
            // close the pipe
            if (-1 == kill(pid, SIGTERM))
            {
                return false;
            }

            int status;
            if (waitpid(pid, &status, 0) == -1)
            {
                perror("Failed to wait for process termination");
                return false;
            }
        }
        pid = -1;
        // is_ready = false;
        return true;
    }

    virtual bool setAsync(bool flag)
    {
        this->is_async = flag;

        if (flag)
        {
            if (ip_pipe[0] >= 0)
            {
                bool res = set_io_flag(ip_pipe[0], O_NONBLOCK);
                if (!res)
                    return false;
            }
            if (ip_pipe[1] >= 0)
            {
                bool res = set_io_flag(ip_pipe[1], O_NONBLOCK);
                if (!res)
                    return false;
            }

            if (op_pipe[0] >= 0)
            {
                bool res = set_io_flag(op_pipe[0], O_NONBLOCK);
                if (!res)
                    return false;
            }

            if (op_pipe[1] >= 0)
            {
                bool res = set_io_flag(op_pipe[1], O_NONBLOCK);
                if (!res)
                    return false;
            }

            if (err_pipe[0] >= 0)
            {
                bool res = set_io_flag(err_pipe[0], O_NONBLOCK);
                if (!res)
                    return false;
            }
            if (err_pipe[1] >= 0)
            {
                bool res = set_io_flag(err_pipe[1], O_NONBLOCK);
                if (!res)
                    return false;
            }
            return true;
        }
        else
        {
            if (ip_pipe[0] >= 0)
            {
                bool res = clear_io_flag(ip_pipe[0], O_NONBLOCK);
                if (!res)
                    return false;
            }
            if (ip_pipe[1] >= 0)
            {
                bool res = clear_io_flag(ip_pipe[1], O_NONBLOCK);
                if (!res)
                    return false;
            }

            if (op_pipe[0] >= 0)
            {
                bool res = clear_io_flag(op_pipe[0], O_NONBLOCK);
                if (!res)
                    return false;
            }

            if (op_pipe[1] >= 0)
            {
                bool res = clear_io_flag(op_pipe[1], O_NONBLOCK);
                if (!res)
                    return false;
            }

            if (err_pipe[0] >= 0)
            {
                bool res = clear_io_flag(err_pipe[0], O_NONBLOCK);
                if (!res)
                    return false;
            }
            if (err_pipe[1] >= 0)
            {
                bool res = clear_io_flag(err_pipe[1], O_NONBLOCK);
                if (!res)
                    return false;
            }
            return true;
        }
        return false;
    }

    virtual bool isAsync()
    {
        return is_async;
    }
    virtual bool init() = 0;
    virtual bool interrupt() = 0;

    IProcess_c()
    {
        if (pipe(ip_pipe) || pipe(op_pipe) || pipe(err_pipe))
        {
            perror("pipe");
        }
    }

    virtual ~IProcess_c()
    {
        close_pipe(ip_pipe[0]);
        close_pipe(ip_pipe[1]);

        close_pipe(op_pipe[0]);
        close_pipe(op_pipe[1]);

        close_pipe(err_pipe[0]);
        close_pipe(err_pipe[1]);

        if (getPid() > 0)
        {
            stop();
        }
    }
};