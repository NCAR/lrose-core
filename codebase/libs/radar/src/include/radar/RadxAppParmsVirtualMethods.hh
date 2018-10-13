#ifdef RADX_APP_PARMS_BASE

virtual void printOperators(void) const = 0;
virtual void setFiltersFromParms(void) = 0;

#else

virtual void printOperators(void) const;
virtual void setFiltersFromParms(void);

#endif
