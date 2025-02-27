#ifndef MESHCLASS
#define MESHCLASS

/**************************************************************************/
/* File:   meshclass.hh                                                    */
/* Author: Joachim Schoeberl                                              */
/* Date:   20. Nov. 99                                                    */
/**************************************************************************/

/*
  The mesh class
*/



enum resthtype { RESTRICTH_FACE, RESTRICTH_EDGE, 
		 RESTRICTH_SURFACEELEMENT, RESTRICTH_POINT, RESTRICTH_SEGMENT };

class HPRefElement;


/// 2d/3d mesh
class Mesh
{
public:
  //  typedef MoveableArray<MeshPoint> T_POINTS;
  //  typedef MoveableArray<Element> T_VOLELEMENTS;
  //  typedef MoveableArray<Element2d> T_SURFELEMENTS;

  typedef ARRAY<MeshPoint,PointIndex::BASE> T_POINTS;
  typedef ARRAY<Element> T_VOLELEMENTS;
  typedef ARRAY<Element2d> T_SURFELEMENTS;


private:
  /// point coordinates
  T_POINTS points;
  /// type of point, is set in calcsurfacesofnode
  //  ARRAY<POINTTYPE,PointIndex::BASE> ptyps;
  /// type of element, set in calcsurfacesofnode
  ARRAY<ELEMENTTYPE> eltyps;

  /// line-segments at edges
  ARRAY<Segment> segments;
  /// surface elements, 2d-inner elements
  T_SURFELEMENTS surfelements;
  /// volume elements
  T_VOLELEMENTS volelements;
  /// points will be fixed forever
  ARRAY<PointIndex> lockedpoints;


  /// surface indices at boundary nodes
  TABLE<int,PointIndex::BASE> surfacesonnode;
  /// boundary edges  (1..normal bedge, 2..segment)
  INDEX_2_CLOSED_HASHTABLE<int> * boundaryedges;
  ///
  INDEX_2_CLOSED_HASHTABLE<int> * segmentht;
  ///
  INDEX_3_CLOSED_HASHTABLE<int> * surfelementht;

  /// faces of rest-solid
  ARRAY<Element2d> openelements;
  /// open segmenets for surface meshing  
  ARRAY<Segment> opensegments;



  /**
     Representation of local mesh-size h
  */
  LocalH * lochfunc;
  ///
  double hglob;
  ///
  ARRAY<double> maxhdomain;
  
  /**
     the face-index of the surface element maps into
     this table.
  */
  ARRAY<FaceDescriptor> facedecoding;

  /// sub-domain materials 
  ARRAY<char*> materials;

  /// Periodic surface, close surface, etc. identifications
  Identifications * ident;


  /// number of vertices (if < 0, use np)
  int numvertices;

  /// geometric search tree for interval intersection search
  Box3dTree * elementsearchtree;
  /// time stamp for tree
  int elementsearchtreets;

  /// element -> face, element -> edge etc ...
  class MeshTopology * topology;
  /// methods for high order elements
  class CurvedElements * curvedelems;
  /// nodes identified by close points 
  class AnisotropicClusters * clusters;

  /// space dimension (2 or 3)
  int dimension;
  
  /// changed by every minor modification (addpoint, ...)
  int timestamp;
  /// changed after finishing global algorithm (improve, ...)
  int majortimestamp;

  /// mesh access semaphor.
  NgMutex mutex;


public:

  // store coarse mesh before hp-refinement
  ARRAY<HPRefElement> * hpelements;
  Mesh * coarsemesh;
  
  
  /// number of refinement levels
  int mglevels;
  /// refinement hierarchy
  ARRAY<INDEX_2,PointIndex::BASE> mlbetweennodes;
  /// parent element of volume element
  ARRAY<int> mlparentelement;
  /// parent element of surface element
  ARRAY<int> mlparentsurfaceelement;



  ///
  Mesh();
  ///
  ~Mesh();

  Mesh & operator= (const Mesh & mesh2);
  
  ///
  void DeleteMesh();
  
  ///
  void ClearSurfaceElements()
  { 
    surfelements.SetSize(0); 
    timestamp = NextTimeStamp();
  }

  ///
  void ClearVolumeElements()
  {
    volelements.SetSize(0); 
    eltyps.SetSize(0);
    timestamp = NextTimeStamp();
  }

  ///
  void ClearSegments()
  { 
    segments.SetSize(0); 
    timestamp = NextTimeStamp();
  }
  
  ///
  bool TestOk () const;


  PointIndex AddPoint (const Point3d & p, int layer = 1);
  int GetNP () const { return points.Size(); }

  MeshPoint & Point(int i) { return points.Elem(i); }
  MeshPoint & Point(PointIndex pi) { return points[pi]; }
  const MeshPoint & Point(int i) const { return points.Get(i); }
  const MeshPoint & Point(PointIndex pi) const { return points[pi]; }

  const MeshPoint & operator[] (PointIndex pi) const { return points[pi]; }
  MeshPoint & operator[] (PointIndex pi) { return points[pi]; }

  /*
  POINTTYPE PointType (int i) const { return ptyps.Get(i); }
  POINTTYPE PointType (PointIndex pi) const { return ptyps[pi]; }
  */
  POINTTYPE PointType (int i) const { return points.Get(i).Type(); }
  POINTTYPE PointType (PointIndex pi) const { return points[pi].Type(); }


  const T_POINTS & Points() const { return points; }
  T_POINTS & Points() { return points; }
  // ARRAY<POINTTYPE,PointIndex::BASE> & PointTypes() { return ptyps; }






  SegmentIndex AddSegment (const Segment & s);
  void DeleteSegment (int segnr)
  {
    segments.Elem(segnr).p1 = PointIndex::BASE-1;
    segments.Elem(segnr).p2 = PointIndex::BASE-1;
  }

  int GetNSeg () const { return segments.Size(); }
  Segment & LineSegment(int i) { return segments.Elem(i); }
  const Segment & LineSegment(int i) const { return segments.Get(i); }

  Segment & LineSegment(SegmentIndex si) { return segments[si]; }
  const Segment & LineSegment(SegmentIndex si) const { return segments[si]; }
  const Segment & operator[] (SegmentIndex si) const { return segments[si]; }
  Segment & operator[] (SegmentIndex si) { return segments[si]; }




  SurfaceElementIndex AddSurfaceElement (const Element2d & el);
  void DeleteSurfaceElement (int eli)
  { 
    surfelements.Elem(eli).Delete();
    surfelements.Elem(eli).PNum(1) = -1; 
    surfelements.Elem(eli).PNum(2) = -1; 
    surfelements.Elem(eli).PNum(3) = -1; 
    timestamp = NextTimeStamp();
  }

  void DeleteSurfaceElement (SurfaceElementIndex eli)
  {
    DeleteSurfaceElement (int(eli)+1);
  }

  int GetNSE () const { return surfelements.Size(); }
  Element2d & SurfaceElement(int i) { return surfelements.Elem(i); }
  const Element2d & SurfaceElement(int i) const { return surfelements.Get(i); }

  Element2d & SurfaceElement(SurfaceElementIndex i)
  { return surfelements[i]; }
  const Element2d & SurfaceElement(SurfaceElementIndex i) const
  { return surfelements[i]; }

  const Element2d & operator[] (SurfaceElementIndex ei) const
  { return surfelements[ei]; }
  Element2d & operator[] (SurfaceElementIndex ei)
  { return surfelements[ei]; }

  
  void GetSurfaceElementsOfFace (int facenr, ARRAY<SurfaceElementIndex> & sei) const;



  ElementIndex AddVolumeElement (const Element & el);

  int GetNE () const { return volelements.Size(); }

  Element & VolumeElement(int i) { return volelements.Elem(i); }
  const Element & VolumeElement(int i) const { return volelements.Get(i); }
  Element & VolumeElement(ElementIndex i) { return volelements[i]; }
  const Element & VolumeElement(ElementIndex i) const { return volelements[i]; }

  const Element & operator[] (ElementIndex ei) const 
  { return volelements[ei]; }
  Element & operator[] (ElementIndex ei)
  { return volelements[ei]; }





  ELEMENTTYPE ElementType (int i) const { return eltyps.Get(i); }
  ELEMENTTYPE ElementType (ElementIndex i) const { return eltyps[i]; }

  const T_VOLELEMENTS & VolumeElements() const { return volelements; }
  T_VOLELEMENTS & VolumeElements() { return volelements; }


  ///
  double ElementError (int eli) const;

  /// 
  void AddLockedPoint (PointIndex pi);
  ///
  void ClearLockedPoints ();

  const ARRAY<PointIndex> & LockedPoints() const
  { return lockedpoints; }

  /// Returns number of domains
  int GetNDomains() const;


  ///
  int GetDimension() const 
  { return dimension; }
  void SetDimension(int dim)
  { dimension = dim; }

  /// sets internal tables
  void CalcSurfacesOfNode ();

  /// additional (temporarily) fix points 
  void FixPoints (const BitArray & fixpoints);

  /**
     finds elements without neighbour and
     boundary elements without inner element.
     Results are stored in openelements.
     if dom == 0, all sub-domains, else subdomain dom */
  void FindOpenElements (int dom = 0);

  
  /**
     finds segments without surface element,
     and surface elements without neighbours.
     store in opensegmentsy
  */
  void FindOpenSegments (int surfnr = 0);
  /**
     remove one layer of surface elements
  */
  void RemoveOneLayerSurfaceElements ();


  int GetNOpenSegments () { return opensegments.Size(); }
  const Segment & GetOpenSegment (int nr) { return opensegments.Get(nr); }
  
  /**
     Checks overlap of boundary
     return == 1, iff overlap
  */
  int CheckOverlappingBoundary ();
  /**
     Checks consistent boundary
     return == 0, everything ok
  */
  int CheckConsistentBoundary () const;

  /*
    checks element orientation
  */
  int CheckVolumeMesh () const;


  /**
     finds average h of surface surfnr if surfnr > 0,
     else of all surfaces.
  */
  double AverageH (int surfnr = 0) const;
  /// Calculates localh 
  void CalcLocalH ();
  ///
  void SetLocalH (const Point3d & pmin, const Point3d & pmax, double grading);
  ///
  void RestrictLocalH (const Point3d & p, double hloc);
  ///
  void RestrictLocalHLine (const Point3d & p1, const Point3d & p2, 
			   double hloc);
  /// number of elements per radius
  void CalcLocalHFromSurfaceCurvature(double elperr);
  ///
  void CalcLocalHFromPointDistances(void);
  ///
  void RestrictLocalH (resthtype rht, int nr, double loch);
  ///
  void LoadLocalMeshSize (const char * meshsizefilename);
  ///
  void SetGlobalH (double h);
  ///
  double MaxHDomain (int dom) const;
  ///
  void SetMaxHDomain (const ARRAY<double> & mhd);
  ///
  double GetH (const Point3d & p) const;
  ///
  double GetMinH (const Point3d & pmin, const Point3d & pmax);
  ///
  LocalH & LocalHFunction () { return * lochfunc; }

  /// Find bounding box
  void GetBox (Point3d & pmin, Point3d & pmax, int dom = -1) const;

  /// Find bounding box of points of typ ptyp or less
  void GetBox (Point3d & pmin, Point3d & pmax, POINTTYPE ptyp ) const;

  ///
  int GetNOpenElements() const
  { return openelements.Size(); }
  ///
  const Element2d & OpenElement(int i) const
  { return openelements.Get(i); }


  /// are also quads open elements
  int HasOpenQuads () const;

  /// split into connected pieces
  void SplitIntoParts ();

  /// 
  void SplitSeparatedFaces ();

  /// Refines mesh and projects points to true surface
  // void Refine (int levels, const CSGeometry * geom);
  

  bool BoundaryEdge (PointIndex pi1, PointIndex pi2) const
  {
    INDEX_2 i2 (pi1, pi2);
    i2.Sort();
    return boundaryedges->Used (i2);
  }

  bool IsSegment (PointIndex pi1, PointIndex pi2) const
  {
    INDEX_2 i2 (pi1, pi2);
    i2.Sort();
    return segmentht->Used (i2);
  }

  SegmentIndex SegmentNr (PointIndex pi1, PointIndex pi2) const
  {
    INDEX_2 i2 (pi1, pi2);
    i2.Sort();
    return segmentht->Get (i2);
  }


  /**
     Remove unused points. etc.
  */
  void Compress ();

  ///
  void Save (const std::string & filename) const;
  ///
  void Load (const std::string & filename);
  ///
  void Merge (const std::string & filename);


  ///
  void ImproveMesh (OPTIMIZEGOAL goal = OPT_QUALITY);

  ///
  void ImproveMeshJacobian (OPTIMIZEGOAL goal = OPT_QUALITY);
  

  /*
#ifdef SOLIDGEOM
  /// old
  void ImproveMesh (const CSGeometry & surfaces, 
		    OPTIMIZEGOAL goal = OPT_QUALITY);
#endif  
  */

  /**
     free nodes in environment of openelements 
     for optimiztion
  */
  void FreeOpenElementsEnvironment (int layers);

  ///
  bool LegalTet (Element & el) const
  {
    if (el.IllegalValid())
      return !el.Illegal();
    return LegalTet2 (el);
  }
  ///
  bool LegalTet2 (Element & el) const;


  ///
  bool LegalTrig (const Element2d & el) const;
  /**
     if values non-null, return values in 4-double array:
     triangle angles min/max, tetangles min/max
     if null, output results on cout
  */
  void CalcMinMaxAngle (double badellimit, double * retvalues = NULL);

  /*
    Marks elements which are dangerous to refine
    return: number of illegal elements
  */
  int MarkIllegalElements ();

  /// orient surface mesh, for one sub-domain only
  void SurfaceMeshOrientation ();

  /// convert mixed element mesh to tet-mesh
  void Split2Tets();


  /// build box-search tree
  void BuildElementSearchTree ();
  /// gives element of point, barycentric coordinates
  int GetElementOfPoint (const Point3d & p,
			 double * lami,
			 bool build_searchtree = 0,
			 const int index = -1) const;

  /// give list of vol elements which are int the box(p1,p2)
  void GetIntersectingVolEls(const Point3d& p1, const Point3d& p2, 
			     ARRAY<int> & locels) const;

  ///
  int AddFaceDescriptor(const FaceDescriptor& fd)
  { return facedecoding.Append(fd); }


  ///
  void SetMaterial (int domnr, const char * mat);
  ///
  const char * GetMaterial (int domnr) const;
    

  ///
  void ClearFaceDescriptors()
  { facedecoding.SetSize(0); }

  ///
  int GetNFD () const
  { return facedecoding.Size(); }

  const FaceDescriptor & GetFaceDescriptor (int i) const
  { return facedecoding.Get(i); }

  ///
  FaceDescriptor & GetFaceDescriptor (int i) 
  { return facedecoding.Elem(i); }

#ifdef NONE
  /*
    Identify points pi1 and pi2, due to
    identification nr identnr
  */
  void AddIdentification (int pi1, int pi2, int identnr);

  int GetIdentification (int pi1, int pi2) const;
  int GetIdentificationSym (int pi1, int pi2) const;
  ///
  INDEX_2_HASHTABLE<int> & GetIdentifiedPoints () 
  { 
    return *identifiedpoints; 
  }

  ///
  void GetIdentificationMap (int identnr, ARRAY<int> & identmap) const;
  ///
  void GetIdentificationPairs (int identnr, ARRAY<INDEX_2> & identpairs) const;
  ///
  int GetMaxIdentificationNr () const
  { 
    return maxidentnr; 
  }
#endif

  /// return periodic, close surface etc. identifications
  Identifications & GetIdentifications () { return *ident; }
  /// return periodic, close surface etc. identifications
  const Identifications & GetIdentifications () const { return *ident; }



  /// find number of vertices
  void ComputeNVertices ();
  /// number of vertices (no edge-midpoints)
  int GetNV () const;
  /// remove edge points
  void SetNP (int np);

  /*
 /// build connected nodes along prism stack
 void BuildConnectedNodes ();
 void ConnectToNodeRec (int node, int tonode, 
 const TABLE<int> & conto);
  */

  bool PureTrigMesh (int faceindex = 0) const;
  bool PureTetMesh () const;


  const class MeshTopology & GetTopology () const
  { return *topology; }
  void UpdateTopology();
  
  class CurvedElements & GetCurvedElements () const
  { return *curvedelems; }

  const class AnisotropicClusters & GetClusters () const
  { return *clusters; }



  int GetTimeStamp() const { return timestamp; }
  void SetNextTimeStamp() 
  { timestamp = NextTimeStamp(); }

  int GetMajorTimeStamp() const { return majortimestamp; }
  void SetNextMajorTimeStamp() 
  { majortimestamp = timestamp = NextTimeStamp(); }


  /// return mutex
  NgMutex & Mutex ()   { return mutex; }

  ///
  friend void OptimizeRestart (Mesh & mesh3d);
  ///
  void PrintMemInfo (std::ostream & ost) const;
  /// 
  friend class Meshing3;
};




#endif
