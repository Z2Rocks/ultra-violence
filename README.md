# ultra-violence
https://youtu.be/TvnYmWpD_T8?is=N5-X-llP4ITYxKNq
MacGyvering our way to a bright future... 
(teach cat-theory to freshmen, plox)
Linear Fibre Extension - LiFE
https://drive.google.com/uc?id=1pSxX9fOGREFLxFecA7as1dS1HtDGKWDj

Cycle extension on raw data, easier to draw it really

this came about by attempting to find a better way of expressing arbitrary boolean functions, think Mobius transformation of a truth table to the algebraic normal form.

Essentially this can be done with any family of composable functions, over any field, and kinda is(?), and by hand it is rather cumbersome.
We concatenate the whole space (with a double), and then scan for structure across its subdivisions.
A straightfoward heuristic provides a hierarchy for detected structure. (this is really the fun part, many different approaches possible)

The functional loop is such:
    start with a vector X
    look for vectors Y which are allowed by your composition function
    record the family of functions which satisfy Fxy(X) = Y
    do that for Fyz(Y) = Z and so forth until you run out of data (or whatever hueristic you choose)
    Compose Fxy . Fyz . ... = Fxyz...
    use the freedom gatherd on the path to generate new data
    link that data in to something 'with hands and feet'