all
===


for f in *.dot; do ../build/main $f; done
for f in *splitting_tree; do sed -i "" -e 's/label="........................................*"/label="truncated"/g' $f; done
for f in *dist_seq; do sed -i "" -e 's/label="........................................*"/label="truncated"/g' $f; done
dot -O -Tpng -Goverlap=false *dist_seq
dot -O -Tpng -Goverlap=false *splitting_tree


graphs
======

dot -O -Tpng -Goverlap=false *dist_seq
dot -O -Tpng -Goverlap=false *splitting_tree

neato -O -Tpng -Goverlap=false *.dot


truncation
==========

sed 's/label="........................................*"/label="truncated"/g' esm-manual-controller.dot.splitting_tree.dot > esm-manual-controller.dot.splitting_tree_truncated.dot

for f in *splitting_tree; do echo $f; sed -i "" -e 's/label="........................................*"/label="truncated"/g' $f; done
for f in *dist_seq; do echo $f; sed -i "" -e 's/label="........................................*"/label="truncated"/g' $f; done


cleaning
========

sed 's/\[label.*<td>I/ \[label="/g;s/<\/td>.*<td>O/ \/ /g;s/<\/td>.*]/"\];/g' 


not needed ctor
===============

partition_refine(Blocks other_blocks){
	auto beg = elements.begin();
	for(auto && block : other_blocks){
		std::copy(block.begin(), block.end(), std::back_inserter(elements));
		blocks.insert(blocks.end(), {beg, std::prev(elements.end())});
		beg = elements.end();
	}
}
