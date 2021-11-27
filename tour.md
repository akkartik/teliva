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

## Task 1: running a Teliva app

Read the first question in [the Readme](https://github.com/akkartik/teliva/blob/main/README.md).
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
app. Spend a few moments scanning this page. All programs consist of data and
_functions_: code that operates on data in some way. Teliva apps always start
by running the special function `main`.

Try browsing to some of the names visible on screen. Don't be afraid to
experiment. The menu at the bottom always shows the hotkeys available to you
at any point in time. Don't worry, everything you do can be undone.

## Task 2: modifying a Teliva app

The [first section of the Lua book](https://www.lua.org/pil/1.html) starts
with this one-line example:

```lua
print("Hello World")
```

Can you figure out where to add this line to the app's code so it's visible
when the app runs?

Start by looking inside `main`. Most Teliva apps tend to share a basic
structure in `main`:
- some initialization, followed by
- a loop that repeatedly updates the screen and then waits for the user to
  press a key

Can you map the lines of `main` to this structure? Which function describes
how the app updates the screen?

Some hints:
- The function responsible for updating the screen is `render`.
- `render` begins by clearing the screen (`window` in Teliva).

After you make a change, can you figure out how to run your program?

Does it do what you expect? Feel free to edit your programs as often as
necessary. Programming is just long sessions of repeatedly editing (`ctrl-e`)
and running (`ctrl-e`) your program.

Once you're happy with your change, try exiting Teliva and restarting it with
the same app. Do you still see your changes?

You could save the file `counter.tlv` anywhere you like at this point. Or
share it with someone else. Everything needed for the app is in that file.` If
you get stuck or have a question, [send it to me!](http://akkartik.name/contact)

## Task 3: variables and arithmetic

Can you figure out how to modify this app to increment by 2 each time you
hit `enter`? Again, don't be afraid to experiment. The menu at the bottom
always shows the hotkeys available to you at any point in time. Don't worry,
everything you do can be undone.

Divide the problem into two parts in your head: where to make your change and
what change to make there. You've already gotten some practice selecting a
place to modify in Task 2. Repeat that process. Go back to the `render` page.
What is the name of the _variable_ (box containing a number) that decides what
number gets printed to screen? Go back to the big picture. Where is this
variable defined? How does it get modified when you press `enter`?

If you're stuck, some short sections from [the Lua book](https://www.lua.org/pil/contents.html)
might help at this point: [getting started](https://www.lua.org/pil/1.html);
[assignment](https://www.lua.org/pil/4.1.html); [what you can do with numbers](https://www.lua.org/pil/3.1.html).

(Buy the Lua book to support the creators of Lua. Teliva is a tiny molehill on
the mountain of awesome that is Lua.)

Some hints:
- The function responsible for processing keystrokes is `update`.
- The variable that tracks what number to print on screen is `n`.
