# A guided tour of Teliva

Teliva is intended to be a safe environment for running shareable little
text-mode Lua apps that are easy to modify. Apps aren't sandboxed yet, but I'm
working on that.

This page is an in-progress guided tour through [Teliva's Readme](https://github.com/akkartik/teliva#readme)
and Lua's reference documentation. We'll start out really slow and gradually
accelerate.

_Prerequisites_

You will need the following to build Teliva:

* A non-mobile computer running some sort of Unix variant. Teliva has been
  tested on Linux, OpenBSD and Mac OS X. (Other BSD variants and Windows
  Subsystem for Linux should require minor changes at most. Please [contact me](http://akkartik.name/contact)
  if you run into issues running Teliva on your computer.)
* [Git](https://git-scm.com).
* A working C toolchain.
* Some fluency in typing commands at the terminal and interpreting their
  output.

## Task 1: getting started

Read the first question in [the Readme](https://github.com/akkartik/teliva/blob/main/README.md)
Try running the commands to download and build Teliva. If you have any trouble
at this point, don't waste _any_ time thinking about it. Just [get in
touch](http://akkartik.name/contact).

Run the simplest example app:

```sh
src/teliva counter.tlv
```

Can you figure out what this app does, what you can do with it? There's a
number on screen. Hit `enter`. The number increments by 1. Hit `ctrl-x` to
exit (press `ctrl` and `x` on your keyboard at the same time).

Try editing the app by hitting `ctrl-e`. You see a "big picture" view of the
app. Can you figure out how to modify this app to increment by 2 each time you
hit `enter`? Don't be afraid to experiment. The menu at the bottom always
shows the hotkeys available to you at any point in time. Don't worry,
everything you do can be undone.

Check out some of the short sections from [the Lua book](https://www.lua.org/pil/contents.html)
that might be useful at this point: [assignment](https://www.lua.org/pil/4.1.html); [what you can do with numbers](https://www.lua.org/pil/3.1.html).

(Buy the Lua book to support the creators of Lua. Teliva is a tiny molehill on
the mountain of awesome that is Lua.)

Once you make a change, can you figure out how to restart the app with your
changes?

After you've made a change, try exiting Teliva and restarting it with the same
app. Do you still see your changes?
