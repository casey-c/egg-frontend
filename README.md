### About
egg-frontend is an experimental testing ground for the frontend portion of [egg](https://github.com/casey-c/egg). It's mostly a place for learning how Qt handles graphics, as well as developing most of the critical algorithms needed to visualize existential graphs. We didn't want to clutter up the egg repo with all the extra junk that comes with learning a new framework for the first time. Our goal is to play around until we've gotten a handle on what exactly we need to implement. By extracting this into a separate project, we can be relatively messy without worrying about breaking any previous code.

### Goals
Eventually, egg-frontend will reach a point where we're happy with its progress to put it into the main egg program. When this happens, combining the two projects will likely not be very straightforward. We're going to have to redesign some of egg itself to handle either a hybrid tree (with nodes pointing to both the frontend tree and backend tree), or combine the functionality of each together into one set of much bigger classes. There may be better solutions out there that we haven't figured out yet, but it's likely that this is the direction we'll eventually be heading.

### License
Like [egg](https://github.com/casey-c/egg) itself, this subproject repo is released under the MIT license. Feel free to do whatever with the code provided.
