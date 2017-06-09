namespace optspp {
  namespace tree {
    template <typename NodeKindEnum>
    struct node {
    
    protected:
      NodeKindEnum node_kind_{NodeKindEnum(0)};
    };
  }
}
