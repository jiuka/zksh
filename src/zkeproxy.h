/* 
 *  zksh - ZooKeeper Shell utils.
 *
 * Copyright (c) 2017 Marius Rieder
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef _ZKEPROXY_H
#define _ZKEPROXY_H

/**
 * \file zkeproxy.h
 * \brief ZKSH Ephemeral proxy definitions
 *
 * To make ephemeral mode working the zksh command forks to background and
 * continues to run till the calling process dies.
 * After the fork the remaining foreground process justs proxies stdin, stdout
 * and stderr until the backgrounded worker signals by SIGUSR1 that it is time
 * to sucessfully end.
 */

/**
 * \brief Initialise the ephemeral proxy configuration
 *
 * This methode takes care of the forking and proxy setup part. Only the child
 * taking care of the ZooKeeper part will return. The master process will
 * continue to run and proxy stdin, stdout and stderr and exit as soon as
 * eigther the child exists or it received a SIGUSR1.
 */
int zksh_eproxy_init();

/**
 * \brief Detaches from the proxy process
 *
 * This methode ensures the proxy process terminates sucessfully.
 */
void zksh_eproxy_detach();

/**
 * \brief Wait for the calling process to die.
 *
 * Send periodicaly a signal 0 to the calling process to verify its existance.
 */
void zksh_eproxy_wait();

#endif /* _ZKEPROXY_H */
