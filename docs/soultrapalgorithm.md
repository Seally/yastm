# Soul Trap Algorithm

The core functionality of YASTM is its soul trap algorithm, which is carefully
crafted to maximize the amount of souls stored inside soul gems given a set of
constraints. There are specialized code paths to deal with each of these
scenarios.

This article details the algorithm in a more human-readable manner. This does
_not_ mean that every human can easily understand it. The main purpose of this
document is to help future readers of the code, primarily myself, understand how
and why the algorithm approaches and solves the problem. If you're effectively
tech-illiterate and come out of this being none the wiser, realize that this
document isn't written for you and you can freely use YASTM without
understanding any of this.

With that said, I will make an effort to help make it _easier_ for
non-programmers to understand things, but know that good questions are mainly
asked by people who already know half the answer.

With that out of the way, let's get started.

## The Lay of the Land

The art of programming is basically the art of crafting a series of translations
to get what you want, based on what you have. It is basically problem solving,
like engineering a recipe for a most delicious cake, except that one who will
execute the recipe is not the baker, but the computer.

One of the key parts of coming up with any kind solution is to know what you
already have. So here's the situation:

* We have a soul gem map from the configuration files. This map allows us to
  know
