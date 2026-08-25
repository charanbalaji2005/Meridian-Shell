import React, { useEffect, useState } from 'react';
import { DocArticle } from '../data/docsContent';
import { NAV_STRUCTURE } from '../data/docsData';
import { Check, Copy, Clock, ArrowLeft, ArrowRight, ShieldCheck, Wrench, Sparkles } from 'lucide-react';

interface DocContentProps {
  article: DocArticle;
  onNavigate: (id: string) => void;
}

export const DocContent: React.FC<DocContentProps> = ({ article, onNavigate }) => {
  const [copiedCodeId, setCopiedCodeId] = useState<string | null>(null);

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
    const preBlocks = document.querySelectorAll('.doc-article-body pre');
    preBlocks.forEach((pre, idx) => {
      if (pre.querySelector('.code-copy-btn')) return;

      const btn = document.createElement('button');
      btn.className = 'code-copy-btn';
      btn.innerHTML = `<span>Copy</span>`;
      btn.onclick = () => {
        const codeText = pre.querySelector('code')?.innerText || pre.textContent || '';
        navigator.clipboard.writeText(codeText.trim());
        btn.innerHTML = `<span style="color: #4ade80;">Copied!</span>`;
        setTimeout(() => {
          btn.innerHTML = `<span>Copy</span>`;
        }, 2000);
      };
      pre.appendChild(btn);
    });
  }, [article]);

  return (
    <article className="fedora-doc-article">
      <header className="article-header">
        <div className="article-meta-row">
          <span className="article-category-badge">{article.category}</span>
          {article.status === 'implemented' && (
            <span className="article-status status-impl">
              <ShieldCheck size={13} /> Implemented & Tested
            </span>
          )}
          {article.status === 'development' && (
            <span className="article-status status-dev">
              <Wrench size={13} /> In Development
            </span>
          )}
          {article.status === 'experimental' && (
            <span className="article-status status-exp">
              <Sparkles size={13} /> Experimental Preview
            </span>
          )}
          <span className="article-updated">
            <Clock size={13} /> {article.lastUpdated}
          </span>
        </div>

        <h1 className="article-title">{article.title}</h1>
        {article.summary && <p className="article-summary">{article.summary}</p>}
      </header>

      <div
        className="doc-article-body"
        dangerouslySetInnerHTML={{ __html: article.body }}
      />

      <footer className="article-footer">
        <div className="article-nav-buttons">
          {prevItem ? (
            <button
              onClick={() => onNavigate(prevItem.id)}
              className="doc-nav-btn prev-btn"
            >
              <ArrowLeft size={16} />
              <div className="nav-btn-text">
                <span className="nav-btn-label">Previous</span>
                <span className="nav-btn-title">{prevItem.title}</span>
              </div>
            </button>
          ) : <div />}

          {nextItem && (
            <button
              onClick={() => onNavigate(nextItem.id)}
              className="doc-nav-btn next-btn"
            >
              <div className="nav-btn-text">
                <span className="nav-btn-label">Next</span>
                <span className="nav-btn-title">{nextItem.title}</span>
              </div>
              <ArrowRight size={16} />
            </button>
          )}
        </div>
      </footer>
    </article>
  );
};
