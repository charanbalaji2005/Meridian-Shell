import React, { useEffect } from 'react';
import { DocArticle } from '../data/docsContent';
import { NAV_STRUCTURE } from '../data/docsNav';
import { Clock, ArrowLeft, ArrowRight, ShieldCheck, Wrench, Sparkles, CircleDashed } from 'lucide-react';

interface DocContentProps {
  article: DocArticle;
  onNavigate: (id: string) => void;
}

export const DocContent: React.FC<DocContentProps> = ({ article, onNavigate }) => {
  // Flatten navigation items to calculate previous & next article
  const allNavItems: { id: string; title: string }[] = [];
  NAV_STRUCTURE.forEach((cat) => {
    cat.items.forEach((item) => {
      allNavItems.push(item);
    });
  });

  const currentIndex = allNavItems.findIndex((item) => item.id === article.id);
  const prevItem = currentIndex > 0 ? allNavItems[currentIndex - 1] : null;
  const nextItem = currentIndex < allNavItems.length - 1 ? allNavItems[currentIndex + 1] : null;

  useEffect(() => {
    // Add copy buttons to rendered pre blocks
    const preBlocks = document.querySelectorAll('.doc-body-content pre');
    preBlocks.forEach((pre) => {
      if (pre.querySelector('.code-copy-btn')) return;

      const btn = document.createElement('button');
      btn.className = 'code-copy-btn';
      btn.innerHTML = `<span>Copy</span>`;
      btn.onclick = () => {
        const codeText = pre.querySelector('code')?.innerText || pre.textContent || '';
        navigator.clipboard.writeText(codeText.trim());
        btn.innerHTML = `<span style="color: #22C55E;">Copied!</span>`;
        setTimeout(() => {
          btn.innerHTML = `<span>Copy</span>`;
        }, 2000);
      };
      pre.appendChild(btn);
    });
  }, [article]);

  return (
    <article className="meridian-article">
      <header className="article-top-header">
        <div className="article-meta-bar">
          <span className="article-cat-tag">{article.category}</span>
          
          {article.status === 'implemented' && (
            <span className="status-tag status-impl">
              <ShieldCheck size={12} /> IMPLEMENTED
            </span>
          )}
          {article.status === 'development' && (
            <span className="status-tag status-dev">
              <Wrench size={12} /> IN DEVELOPMENT
            </span>
          )}
          {article.status === 'experimental' && (
            <span className="status-tag status-exp">
              <Sparkles size={12} /> EXPERIMENTAL
            </span>
          )}
          {article.status === 'planned' && (
            <span className="status-tag status-plan">
              <CircleDashed size={12} /> PLANNED
            </span>
          )}

          <span className="article-date">
            <Clock size={12} /> {article.lastUpdated}
          </span>
        </div>

        <h1 className="article-main-title">{article.title}</h1>
        {article.summary && article.id !== 'intro' && (
          <p className="article-lead-summary">{article.summary}</p>
        )}
      </header>

      <div
        className="doc-body-content"
        dangerouslySetInnerHTML={{ __html: article.body }}
      />

      <footer className="article-bottom-footer">
        <div className="article-nav-pager">
          {prevItem ? (
            <button
              onClick={() => onNavigate(prevItem.id)}
              className="pager-btn prev-pager"
            >
              <ArrowLeft size={16} />
              <div className="pager-text-block">
                <span className="pager-dir">Previous</span>
                <span className="pager-name">{prevItem.title}</span>
              </div>
            </button>
          ) : <div />}

          {nextItem && (
            <button
              onClick={() => onNavigate(nextItem.id)}
              className="pager-btn next-pager"
            >
              <div className="pager-text-block">
                <span className="pager-dir">Next</span>
                <span className="pager-name">{nextItem.title}</span>
              </div>
              <ArrowRight size={16} />
            </button>
          )}
        </div>
      </footer>
    </article>
  );
};
