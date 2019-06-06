extracting code from Soloii ...
remove the use of seds, dgi data structures; instead, pass args
break dgi into separate chunks of info

pass around the boundary structures? rather than keep them in a global, like seds?

remove the command structure?
keep the boundary data structures ... 
- BoundaryStuff *sebs 
- boundary point management
- OneBoundary
- BoundaryPoint ...
- PointInSpace 


NOTE: many pieces of info are passed around inside global structures
For example, a value inside a global structure is set in one
function, then in another function, the value is retrieved.
This is done frequently instead of passing the info as 
arguments and return values.
