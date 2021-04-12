/*
 * This file is part of the EMBTOM project
 * Copyright (c) 2018-2020 Thomas Willetal
 * (https://github.com/embtom)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//******************************************************************************
// Header

#include <FdSet.h>
#include <sstream> //std::stringstream
#include <string.h> //strerror
#include <errno.h> //errno
#include <unistd.h> //write, close
#include <sys/epoll.h> //epoll
#include <error_msg.hpp> //buildErrorMessage
#include <list>

namespace utils
{
struct event_data
{
   int fd;
   FdSet::Callback cb;
};

//*****************************************************************************
//! \brief FdSet
//!
class FdSetPrivate
{

public:
   explicit FdSetPrivate();
   ~FdSetPrivate() noexcept;
   void AddFd(int fd, FdSet::Callback cb);
   void RemoveFd(int fd);
   FdSetRetval Select(FdSet::Callback cb) const;
   void UnBlock() const;

private:
   std::list<event_data>   m_eventData;
   int                     m_epollFd {0};
   int                     m_unBlockFd[2] {0};
};
} //utils

using namespace utils;

//*****************************************************************************
// Method definitions "FdSetPrivate"

FdSetPrivate::FdSetPrivate()
{
   if(pipe(m_unBlockFd) == -1)
   {
      throw std::runtime_error(utils::buildErrorMessage("FdSet::", __func__, " Failed to create pipe with error: ", strerror(errno)));
   }

   int ret = epoll_create1(EPOLL_CLOEXEC);
   if (ret == -1)
   {
      close(m_unBlockFd[0]);
      close(m_unBlockFd[1]);
      throw std::runtime_error(utils::buildErrorMessage("FdSetPrivate::", __func__, " Failed to open epoll with error: ", strerror(errno)));
   }
   m_epollFd = ret;
   AddFd(m_unBlockFd[0], nullptr);
}

FdSetPrivate::~FdSetPrivate() noexcept
{
   UnBlock();
   close(m_epollFd);
   close(m_unBlockFd[0]);
   close(m_unBlockFd[1]);
}

void FdSetPrivate::AddFd(int fd, FdSet::Callback cb)
{
   m_eventData.emplace_back(event_data{.fd = fd, .cb = cb});

   epoll_event epev = {0};
   epev.events = EPOLLIN;
   epev.data.ptr = &m_eventData.back();

   if(epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &epev) == -1)
   {
      throw std::runtime_error(utils::buildErrorMessage("FdSetPrivate::",__func__," Failed to add fd to epoll: ", strerror(errno)));
   }
}

void FdSetPrivate::RemoveFd(int fd)
{
   auto it = std::find_if(m_eventData.begin(), m_eventData.end(), [&fd] (event_data& elm)
   {
      if(elm.fd == fd) {
         return true;
      }
      else {
         return false;
      }
   });

   if (it == m_eventData.end()) {
      throw std::runtime_error(utils::buildErrorMessage("FdSetPrivate::",__func__," Fd to remove not available"));
   }

   if(epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
   {
      throw std::runtime_error(utils::buildErrorMessage("FdSetPrivate::",__func__," Failed to remove fd to epoll: ", strerror(errno)));
   }

   m_eventData.erase(it);
}

FdSetRetval FdSetPrivate::Select(FdSet::Callback cb) const
{
   int ret;
   epoll_event epev[5] = {0};
   FdSetRetval ERet = FdSetRetval::OK;

   do {
	   ret = epoll_wait(m_epollFd, &epev[0], 5, -1);
		if ((ret == -1) && (errno != EINTR))
      {
         throw std::runtime_error(utils::buildErrorMessage("FdSetPrivate::",__func__," Failed to epoll_wait: ", strerror(errno)));
      }

      for(int i = 0; i < ret; i++)
      {
         event_data *pEventData = static_cast<event_data*>(epev[i].data.ptr);
         if(pEventData->fd == m_unBlockFd[0]) {
            ERet = FdSetRetval::UNBLOCK;
            continue;
         }

         if(cb) {
            cb(pEventData->fd);
         }
         if(pEventData->cb) {
            pEventData->cb(pEventData->fd);
         }
      }
	} while ((ret == -1) && (errno == EINTR));

   return ERet;
}

void FdSetPrivate::UnBlock() const
{
   int dummy = 1;
   if(write(m_unBlockFd[1],&dummy, sizeof(dummy)) == -1)
   {
      throw std::runtime_error(utils::buildErrorMessage("FdSetPrivate::",__func__," Failed to write to unblock pipe: ", strerror(errno)));
   }
}

//*****************************************************************************
// Method definitions "FdSet"
FdSet::FdSet() :
   m_pPrivate(std::make_unique<FdSetPrivate>())
{ }

FdSet::~FdSet() = default;

void FdSet::AddFd(int fd, Callback cb)
{
   if (!cb) {
      throw std::runtime_error(utils::buildErrorMessage("FdSet::",__func__," No callback passed "));
   }
   m_pPrivate->AddFd(fd, cb);
}

void FdSet::AddFd(int fd)
{
   m_pPrivate->AddFd(fd, nullptr);
}


void FdSet::RemoveFd(int fd)
{
   m_pPrivate->RemoveFd(fd);
}

FdSetRetval FdSet::Select() const
{
   return m_pPrivate->Select(nullptr);
}

FdSetRetval FdSet::Select(Callback cb) const
{
   return m_pPrivate->Select(cb);
}

void FdSet::UnBlock() const
{
   m_pPrivate->UnBlock();
}