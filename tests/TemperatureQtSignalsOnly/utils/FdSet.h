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

#include <vector> // std::vector
#include <functional> //std::function
#include <memory>

namespace utils
{

class FdSetPrivate;

//*****************************************************************************
//! \brief FdSet
//!
enum class FdSetRetval
{
   OK,
   UNBLOCK
};


class FdSet
{
public:
   using Callback = std::function<void(int fd)>;

   FdSet(const FdSet&)            = delete;
   FdSet& operator=(const FdSet&) = delete;
   FdSet(FdSet&&)                 = default;
   FdSet& operator=(FdSet&&)      = default;

   explicit FdSet();
   ~FdSet();

   void AddFd(int fd, Callback cb);
   void AddFd(int fd);
   void RemoveFd(int fd);
   FdSetRetval Select() const;
   FdSetRetval Select(Callback cb) const;
   void UnBlock() const;
private:
   std::unique_ptr<FdSetPrivate>   m_pPrivate;
};

} //util