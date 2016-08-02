#ifndef	__GLUTILS_H
#define	__GLUTILS_H

/*
 * glutils.h
 * 
 * General radar related gl utilities,  e.g. Lat/Long/Ht transformations
 */

void DumpCurrentMatrix(); // dumps the current gl matrix;

void GlSetOriginLatLong(LatLongHt *Dest);
void GlSetOriginLatLong(float Lat, float Long, float Ht);
void GlSetOriginRdrSite(int SiteDest);
void GlUnsetOriginLatLong(LatLongHt *Org);
void GlUnsetOriginRdrSite(int SiteOrg);
void GlMvOriginLatLong(LatLongHt *Org, LatLongHt *Dest);
void GlMvOriginRdrSite(LatLongHt *Org, int SiteDest);

#endif	/* __GLUTILS_H */
