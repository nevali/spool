Spool is a system for processing batches of assets.

*Current status*: Skeleton code. Does not do anything useful.

Spool is designed to:

* Retrieve assets from a source, possibly with a metadata sidecar
* Identify the type of the asset
* Assign a unique identifier (a UUID) to the asset
* Transfer the asset (and sidecar) to a storage pool
* Perform transformations upon the asset (defined by a set of recipes)

