#使用后缀数组为语料库中所有子串计算词频和文档频率
Mikio Yamamoto  University of Tsukuba
Kenneth W. Church   AT&T Labs - Research

二元文法和三元文法被普遍使用在统计自然语言处理中。本篇论文将探讨更长的n元文法的技术。后缀数组第一次被引入进来计算子串(n-gram)的频率，以及子串在长度为N的序列语料库中的位置。为了计算所有语料库中的N(N+1)/2个子串的频率， 子串们被组织成可控数量的等价类。通过这种方式，不可计算的子串就变成了可以计算的等价类。 本篇论文展现了计算所有子串词频(term frequency tf)和文档频率(doctument frequency df)的算法和代码。这两个大型语料库分别是英文语料库，5千万词的“华尔街日报以及日文语料库2亿1千6百万字的“每日新闻”。

文章的第二部分使用这些频率寻找有意义的子串。 字典编撰者一直对具有高互信息(Mutual Information)的n元文法感兴趣， 联合词频高于偶然预期的组合。在信息检索任务中，Residual Inverse Document Frequency （RIDF）剩余逆文档频率将文档频率与不同的机会概念进行比较，突出显示用于信息检索的技术术语，名称和良好关键字。 在日文单词识别任务中，互信息与剩余逆文档频率的组合比单独使用其一有更好的效果。

##1 简介
后缀数组将用于计算大型语料库中所有ngram的一些感兴趣的统计数据，包括词频和文档频率。词频(tf)和文档频率(df)以及互信息(MI)逆文档频率(IDF)等函数在基于语料库曷信息检索(IR)的文献中得到了相当多的关注。词频是基于语料库的自然语言处理(NLP)中的标准频率概念。文档频率是包含该术语的文档的数量。 词频是一个0到N之间的整数， N是语料库的大小。 文档频率是一个0到D之间的整数， D是语料库中文档的数量。这些统计数据通常通过bigrams和trigrams等短数据计算得出。 本篇文章将展示如何使用更长的ngrams，包括百万甚至十亿ngrams。
在基于语料库的自然语言处理， 词频通常转换为概率，使用最大似然估计(MLE), Good-Turing方法或删除插值法。这些概率用于语音识别的噪声信道应用中，将可能的序列与不可能的序列区分开，从而减少声学识别器的搜索空间。 在信息检索中，文档频率被转换为逆文档频率(IDF),在术语加权中起着重要作用。
IDF(t)=-log(df(t)/D)
IDF(t) 可以解释为文档包含术语t所具有的信息位数， 稀有术语比常用术语贡献更多位。
相互信息（MI）和剩余IDF（RIDF）都将tf和df与偶然预期的结果进行比较，使用两种不同的偶然概念。 MI将整体的频率与部件的频率进行比较。 较大的MI表示与组合性的较大偏差。 RIDF（Church and Gale，1995）将术语与文档的分布与随机（无意义）术语的预期进行了比较。 用于信息检索的良好关键词倾向于挑选相对少量的文档，这与根据泊松分布在相对大量的文档中找到的随机项不同。
本文的其余部分分为两部分。第2节描述了算法以及用于计算两个大型语料库中所有子串的术语频率和文档频率的代码，华尔街日报的5000万字的英语语料库，以及每日新闻2.16亿字符的日语语料库。
第3节使用这些频率来找到“有趣的”子串，其中被称为“有趣”的子串取决于应用。 MI找到词典编纂的短语，一般词汇的分布远非成分，而RIDF挑选出对信息检索有用的技术术语，名称和关键词，其在文档上的分布远非均匀或泊松。这些观察结果可能对日本形态特别有用，将日文字符分割成单词。 MI和RIDF中较高的字符序列比单独一个中较高的序列更可能是单词，这两个序列都比两个中较高的序列更可能。

##2 计算所有子串的TF和DF
###2.1 后缀数组
本节将介绍一种基于后缀数组的算法，用于在O（N logN）时间内为语料库中的所有子串计算tf和df以及这些量的许多函数，即使在N（N + 1）/ 2个这样的子串中也是如此。 大小为N的语料库。该算法将N（N + 1）/ 2个子串分组为最多2N - 1个等价类。 通过以这种方式对子串进行分组，许多感兴趣的统计数据可以在相对较少数量的类上计算，这是可管理的，而不是在二次数的子串上计算，这可能是令人望而却步的。

引入了后缀数组数据结构（Manber和Myers，1990）作为数据库索引技术。后缀数组可以看作是后缀树的紧凑表示（McCreight，1976; Ukkonen，1995），这是一种在过去三十年中得到广泛研究的数据结构。有关后缀树的全面介绍，请参阅（Gusfield，1997）。 Hui（1992）展示了如何使用广义后缀树计算所有子串的df。后缀数组相对于后缀树的主要优点是空间。后缀树（但不是后缀数组）的空间要求随字母大小增加：O（N |Σ|）空间，其中|Σ|是字母大小。对字母大小的依赖对日语来说是一个严重的问题。 Manber和Myers（1990）报道，即使在相对较小的字母大小（|Σ| = 96）的情况下，后缀数组在空间上的效率也比后缀树高一个数量级。对于诸如日语（和英语单词）之类的较大字母表，后缀数组优于后缀树的优点变得更加重要。

后缀数组数据结构便于计算长序列（语料库）中子串（ngram）的频率和位置。 早期工作的动机是生物学应用，如DNA序列的匹配。 后缀阵列与PAT阵列密切相关，其部分原因是滑铁卢大学的一个项目将牛津英语词典与CD-ROM上的索引一起分发.PAT阵列也受到信息检索应用的推动（Gaston H. Gonnet和Snider，1992）。 Nagao和Mori（1994）提出了类似的数据结构来处理日文文本。

在每种情况下，字母表大小都有很大差异。 DNA有一个相对较小的字母表，只有4个字符，而日语有一个相对较大的字母表，超过5000个字符。 这些方法在字母大小上自然缩放。 在第3节中，数据结构应用于大型英语文本语料库，其中假定字母表是所有英语单词的集合，即无界集合。 有时候假设较大的字母比较小的字母更具挑战性，但具有讽刺意味的是，它可能恰恰相反，因为字母表的大小与有意义/有趣的子串的长度之间通常存在反比关系。

本节首先回顾后缀数组的构造以及它们如何用于计算序列中子字符串的频率和位置。 然后我们将展示如何应用这些方法不仅可以查找特定子字符串的频率，还可以查找所有子字符串的频率。 最后，将这些方法推广到计算文档频率以及术语频率。

后缀数组s是所有N个后缀的数组，按字母顺序排序。 后缀s[i]，也称为半无限字符串，是从语料库中的位置i开始并继续到语料库末尾的字符串。 在实际实现中，通常表示为4字节整数，i。 通过这种方式，一个小的（恒定的）空间用于表示一个非常长的子字符串，人们可能认为它需要N个空间。
下面给出的算法suffix_array 输入是一个语料库及其长度N，输出是后缀数组s。

>
suffix_array ← function(corpus, N){
  Let s be a vector of the integers from 0 to N − 1 which denote suffixes
  starting from s[i] on the corpus.
  Sort s in alphabetical order of suffixes denoted by each integer.
  Return s.
}

算法的C语言实现

```
char *corpus;
int suffix_compare(int *a, int *b) {
  return strcmp(corpus+*a, corpus+*b);
}
int *suffix_array(int n) {
  int i, *s = (int *)malloc(n*sizeof(int));
  for(i=0; i < n; i++) s[i] = i; /* initialize */
  qsort(s, n, sizeof(int), suffix_compare); /* sort */
  return s;
}
```

图1和图2展示了一个简单的例子：语料库是(“to be or not to be”)长度是18。（13个字母加上5个空格）C程序（上面）首先为后缀数组分配内存（18个整数，每个4字节）。 后缀数组初始化为0到17之间的整数。最后，后缀数组按字母顺序排序。 初始化后的后缀数组如图1所示。排序后的后缀数组如图2所示。

input corpus: "to be or not to be"
|  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  | 10  |  11 |  12 |  13 | 14  | 15  | 16  | 17  |     |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
|  t  |  o  |  _  |  b  |  e  |  _  |  o  |  r  |  _  |  n  |  o  |  t  |  _  |  t  |  o  |  _  |  b  |  e  | null|


|Initialized Suffix Array|||Suffixes denoted by s[i]|
|:--:|:--:|:--:|:--:|
|S[0]|  0  |    |to_be_or_not_to_be|
|S[1]|  1  |    |o_be_or_not_to_be|
|S[2]|  2  |    |_be_or_not_to_be|
|S[3]|  3  |    |be_or_not_to_be|
|... | ... |==> |...|
|S[13]| 13 |    |to_be|
|S[14]| 14 |    |o_be|
|S[15]| 15 |    |_be|
|S[16]| 16 |    |be|
|S[17]| 17 |    |e|

图1 展示了后缀数组S，刚刚初始化但是没有排序。后缀数组的每个元素s[i]是表示后缀或半无限字符串的整数，从预料库中的位置i开始并延伸到语料库的末尾。

图2显示了如何使用该过程来计算语料库中“待成为或不成为”的术语“待定”的频率和位置。 如图所示，s [i = 16]是以“to be”开头的第一个后缀，s [j = 17]是从该术语开始的最后一个后缀。 因此，tf（“to be”）= 17 - 16 + 1 = 2.此外，该术语出现在（“to be”）= {s [16]，s [17]} = {13,0}的位置， 而且只有这些职位。 类似地，子串“to”具有相同的tf和位置，子串也是“to”和“to b。”虽然可能有N（N + 1）/ 2种方式来选择i和j，但它会转向 在计算所有子串的tf时，我们只需要考虑2N - 1。

Nagao和Mori（1994）使用这个程序，将其成功地应用于日语文本的大型语料库。他们报告它需要O（N logN）时间，假设排序步骤执行O（N logN）比较，并且每个比较可以在O（1）时间内完成。虽然这些通常是合理的假设，但我们发现如果语料库包含长重复的子串（例如，重复的文章），就像我们的英语语料库那样（Paul和Baker，1992），那么排序可以消耗二次时间，因为每次比较都可以N次。像Nagao和Mori（1994）一样，我们也能够将这个程序非常成功地应用到我们的日语语料库中，但是对于英语语料库，在50小时的cpu时间之后，我们放弃并转向Manber和Myers（1990）算法，1只花了两个小时。 Manber和Myers的算法使用一些聪明但难以描述的技术来实现O（N logN）时间，即使对于具有长重复子串的语料库也是如此。对于否则会消耗二次时间的语料库，Manber和Myers的算法非常值得，但除此之外，上述过程更简单，甚至可以更快一些。

|Suffix Array|||Suffixes denoted by s[i]|
|:--:|:--:|:--:|:--:|
|S[0]|  15 |    |_be|
|S[1]|  2  |    |_be_or_not_to_be|
|S[2]|  8  |    |_not_to_be|
|S[3]|  5  |    |_or_not_to_be|
|S[4]|  12 |    |_to_be|
|S[5]|  16 |    |be|
|S[6]|  3  |    |be_or_not_to_be|
|S[7]|  17 |    |e|
|S[8]|  4  |    |e_or_not_to_be|
|S[9]|  9  |    |not_to_be|
|S[10]| 14 |    |o_be|
|S[11]| 1  |    |o_be_or_not_to_be|
|S[12]| 6 |     |or_not_to_be|
|S[13]| 10 |    |ot_to_be|
|S[14]| 7  |    |r_not_to_be|
|S[15]| 11 |    |t_to_be|
|S[16]| 13 |    |to_be|
|S[17]| 0  |    |to_be_or_not_to_be|

图2 展示了排序后的后缀数组S，对s中的整数进行排序，以便半无限字符串现在按字母顺序排列。

“to_be_or_not_to_be”的例子使用标准英文字母（每个字符一个字节）。 如上所述，后缀数组可以以简单的方式推广，以使用较大的字母表，例如日语（通常每个字符两个字节）。 在第3节中，我们使用开放式英语单词作为字母表。 每个标记（英文单词）表示为符号表（字典）的4字节指针。 例如，“要成为或不成为”的语料库被标记为6个标记：“to”，“be”，“or”，“not”，“to”和“be”，其中每个标记是 表示为字典的4字节指针。

###2.2 最长公共前缀(LCPs)

后缀数组利用辅助数组来存储LCP（最长公共前缀）。 lcp数组包含N + 1个整数。 每个元素lcp[i]表示s[i-1]和s[i]之间的公共前缀的长度。 我们用零填充lcp向量（lcp [0] = lcp [N] = 0）以简化下一节中某些概念的代码和定义。 填充避免了测试某些结束条件的需要。

图3显示了后缀数组“to_or_not_to_be”的lcp向量。例如，由于s[10]和s[11]都以子字符串“o be”开头，lcp[11]被设置为4，最长公共前缀的长度。 Manber和Myers（1990）在他们的O（P + logN）算法中使用lcp向量来计算长度为N的子串的长度为P的子串的频率和位置。他们表明lcp向量可以在O中被置换（N logN）时间。 当语料库包含长重复的子串时，这些算法比明显简单的实现快得多，但对于许多语料库，避免二次行为所需的复杂性是不必要的。

|Suffix Array|||Suffixes denoted by s[i]||lcp vector|
|--:|:--:|:--:|:--|--:|:--:|
|S[0]|  15 |    |_be|                   lcp[0] | 0  |
|S[1]|  2  |    |_be_or_not_to_be|      lcp[1] | 3  |
|S[2]|  8  |    |_not_to_be|            lcp[2] | 1  |
|S[3]|  5  |    |_or_not_to_be|         lcp[3] | 1  |
|S[4]|  12 |    |_to_be|                lcp[4] | 1  |
|S[5]|  16 |    |be|                    lcp[5] | 0  |
|S[6]|  3  |    |be_or_not_to_be|       lcp[6] | 2  |
|S[7]|  17 |    |e|                     lcp[7] | 0  |
|S[8]|  4  |    |e_or_not_to_be|        lcp[8] | 1  |
|S[9]|  9  |    |not_to_be|             lcp[9] | 0  |
|S[10]| 14 |    |o_be|                 lcp[10] | 0  |
|S[11]| 1  |    |o_be_or_not_to_be|    lcp[11] | 4  |
|S[12]| 6 |     |or_not_to_be|         lcp[12] | 1  |
|S[13]| 10 |    |ot_to_be|             lcp[13] | 1  |
|S[14]| 7  |    |r_not_to_be|          lcp[14] | 0  |
|S[15]| 11 |    |t_to_be|              lcp[15] | 0  |
|S[16]| 13 |    |to_be|                lcp[16] | 1  |
|S[17]| 0  |    |to_be_or_not_to_be|   lcp[17] | 5  |

图3 最长公共前缀是N + 1个整数的向量。 lcp[i]表示后缀s[i-1]和后缀s[i]之间的公共前缀的长度。 因此，例如，s[10]和s[11]共享4个字符的公共前缀，因此lcp[11] = 4.公共前缀由后缀数组中的虚线突出显示。 后缀数组与上图中的相同。

###2.3 子串的等价类

到目前为止，我们已经看到如何计算单个ngram的tf，但是我们如何计算所有ngram的tf和df？ 如上所述，N（N + 1）/ 2个子串将被聚类成相对较少数量的类，然后统计数据将在类而不是子串上计算，这将是令人望而却步的。 通过四个属性可以将子串上的计算减少到类上的计算。 类中的子串都共享相同的tf（属性1）和相同的df（属性2）。 而且，每个子字符串都是某个类的成员（属性3b）。 最后，可管理的类数量远小于子串的数量（属性4）。

类是按间隔定义的。 令（i，j）是后缀数组上的区间，s [i]，s [i + 1]，...，s [j]。 构造类以使类中的子串是等价的，至少相对于tf（属性1）和df（属性2）。 Class（i，j）是子串的集合，它在区间内开始每个后缀，并且在区间之外没有子串。 物业1紧随其后。 也就是说，类（i，j）中的每个子字符串都具有tf = j -i + 1，因此，如果一个类中有两个子字符串，它们将共享相同的术语频率。

类中的子串集可以从lcp向量构造：class（i，j）= {s [i] m | max（lcp [i]，lcp [j + 1]）<m≤min（lcp [i + 1]，lcp [i + 2]，...，lcp [j]）}，其中s [i] m表示 后缀s [i]的前m个字符。 我们将lcp [i]和lcp [j + 1]称为绑定lcps和lcp [i + 1]，lcp [i + 2]，...，lcp [j]作为内部lcps。上面的等式可以写成：
class(i, j) = {s[i]m|LBL(i, j)< m ≤ SIL(i, j)},
其中LBL (longest bounding lcp)是
LBL(i, j) = max(lcp[i], lcp[j + 1]),
SIL (shortest interior lcp)是
SIL(i, j) = min(lcp[i + 1], lcp[i+ 2], ..., lcp[j]).