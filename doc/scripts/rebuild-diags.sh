SCRIPT_DIR=$(dirname $0)
SFL_ROOT=$(realpath $SCRIPT_DIR/../../)

plantuml -tsvg $SFL_ROOT/doc/diagrams/*.puml