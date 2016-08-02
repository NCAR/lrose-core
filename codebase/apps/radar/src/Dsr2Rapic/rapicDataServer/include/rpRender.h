/*
  
  rpRender.h

  Base rapic render class

*/

class rpRender
{
 public:
  rpRender() { enabled = true; };
  virtual ~rpRender() {} ;
  bool Enabled() { return enabled; };
  void setEnabled(bool state = true) { enabled = state; };
  virtual void render() {}; // render origin must be set to default, e.g. centre of earth
  // default co-ordinate system is km XYZ WRT centre of earth
 protected:
  bool enabled;
};
