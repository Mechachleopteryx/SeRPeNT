#include <diffproc/diffprocio.h>

/*
 * Generate a vector of gaussian white noise adjusted to a given mean and variance.
 *
 * Vector is generated by the Central Limit Thorem Method, that states that
 * the sum of N randoms will approach normal distribution as N approaches infinity.
 * N is recommended to be >= 20.
 *
 * @reference Jeruchim, "Simulation of communication systems"
 *            1st Ed. Springer, 1992.
 *
 * @arg const double signal[]
 *   Deterministic signal
 * @arg const double mean
 *   Mean value which the vector will be adjusted for
 * @arg const double variance
 *   Variance value which the vector will be adjusted for
 * @arg double* noise
 *   The Gaussian white noise vector
 * @arg int nlen
 *   Length of the gaussian noise vector
 */
void pgnoise(const double signal[], const double mean, const double variance, double* noise, int nlen)
{
  double x = 0;
  int i, j;

  for (i = 0; i < nlen; i++) {
    for (j = 0; j < MAX_GNOISE_N; j++) {
      double u = (double) rand() / (double)RAND_MAX;
      x += u;
    }

    // for uniform randoms in [0,1], mu = 0.5 and var = 1/12
    x = x - MAX_GNOISE_N / 2;        // set mean to 0
    x = x * sqrt(12 / MAX_GNOISE_N); // adjust variance to 1

    // modify x in order to have a particular mean and variance
    noise[i] = mean + sqrt(variance) * x;
  }
}


/*
 * next_diffproc_feature
 *
 * @see include/diffproc/diffprocio.h
 */
int next_diffproc_feature(FILE* bedf, feature_struct_diffproc* feature)
{
  char *line = NULL;
  char *token, *index;
  size_t len = 0;
  ssize_t read;

  read = getline(&line, &len, bedf);

  if (read < 0) {
    free(line);
    return(0);
  }

  if ((token = strtok(line, "\t")) == NULL) {
    free(line);
    return(-1);
  }
  strncpy(feature->chromosome, token, MAX_FEATURE);

  if ((token = strtok(NULL, "\t")) == NULL) {
    free(line);
    return(-1);
  }
  feature->start = atoi(token);

  if ((token = strtok(NULL, "\t")) == NULL) {
    free(line);
    return(-1);
  }
  feature->end = atoi(token);

  if ((token = strtok(NULL, "\t")) == NULL) {
    free(line);
    return(-1);
  }
  strncpy(feature->name, token, MAX_FEATURE);

  if ((token = strtok(NULL, "\t")) == NULL) {
    free(line);
    return(-1);
  }
  feature->score = atof(token);

  if ((token = strtok(NULL, "\t")) == NULL) {
    free(line);
    return(-1);
  }
  if (strcmp(token, "+") == 0) feature->strand = FWD_STRAND;
  else feature->strand = REV_STRAND;

  if ((token = strtok(NULL, "\t")) == NULL) {
    free(line);
    return(-1);
  }
  if (strcmp(token, "NOVEL") == 0) feature->status = 0;
  else feature->status = 1;

  if ((token = strtok(NULL, "\t")) == NULL) {
    free(line);
    return(-1);
  }
  index = token;
  while(index[1]) ++index;
  if (*index == '\n') *index = '\0';
  feature->cluster = atoi(token);

  free(line);

  return(1);
}

/*
 * next_diffproc_profile
 *
 * @see include/diffproc/diffprocio.h
 */
int next_diffproc_profile(FILE* fp, profile_struct_diffproc* profile)
{
  char *line = NULL;
  char *token, *feature, *cline;
  size_t len = 0;
  ssize_t read;
  int i;

  read = getline(&line, &len, fp);

  if (read < 0) {
    free(line);
    return(0);
  }

  cline = (char*) malloc((strlen(line) + 1) * sizeof(char));
  strcpy(cline, line);

  if ((token = strtok(line, "\t")) == NULL) {
    free(cline);
    free(line);
    return(-1);
  }

  if ((feature = strtok(token, ":")) == NULL)  {
    free(cline);
    free(line);
    return(-1);
  }
  strcpy(profile->chromosome, feature);

  if ((feature = strtok(NULL, "-")) == NULL)  {
    free(cline);
    free(line);
    return(-1);
  }
  profile->start = atoi(feature);

  if ((feature = strtok(NULL, ":")) == NULL)  {
    free(cline);
    free(line);
    return(-1);
  }
  profile->end = atoi(feature);
  profile->length = profile->end - profile->start + 1;

  if ((feature = strtok(NULL, ":")) == NULL)  {
    free(cline);
    free(line);
    return(-1);
  }

  if (strcmp(feature, "+") == 0)
    profile->strand = FWD_STRAND;
  else if (strcmp(feature, "-") == 0)
    profile->strand = REV_STRAND;
  else {
    free(cline);
    free(line);
    return(-1);
  }

  profile->profile = (double*) malloc((profile->end - profile->start + 1) * sizeof(double));

  if ((token = strtok(cline, "\t")) == NULL) {
    free(cline);
    free(line);
    return(-1);
  }

  for (i = 0; i < (profile->end - profile->start + 1); i++) {
    if ((token = strtok(NULL, "\t")) != NULL)
      profile->profile[i] = (double) atof(token);
    else {
      free(cline);
      free(line);
      return(-1);
    }
  }

  strncpy(profile->annotation, "unknown", MAX_FEATURE);
  pgnoise(profile->profile, gsl_stats_mean(profile->profile, 1, profile->length), gsl_stats_variance(profile->profile, 1, profile->length), profile->noise, MAX_PROFILE_LENGTH);
  profile->cluster = -1;
  profile->position = -1;
  profile->differential = 0;
  profile->partner = NULL;

  free(cline);
  free(line);
  return(1);
}

/*
 * find_clusters
 *
 * @see include/diffproc/diffprocio.h
 */
int find_clusters(FILE* bedf)
{
  feature_struct_diffproc feature;
  int result;
  int ncl = -1;

  while((result = next_diffproc_feature(bedf, &feature) > 0)) {
    if (ncl < feature.cluster)
      ncl = feature.cluster;
  }

  return(ncl);
}

/*
 * allocate_clusters
 *
 * @see include/diffproc/diffprocio.h
 */
void allocate_clusters(FILE* bedf, int* profiles_per_cluster)
{
  feature_struct_diffproc feature;
  int result;

  while((result = next_diffproc_feature(bedf, &feature) > 0))
    profiles_per_cluster[feature.cluster - 1]++;
}
